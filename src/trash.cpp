#include "trashcli.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>
#include <system_error>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#include <unistd.h>
#include <sys/stat.h>
#include <sqlite3.h>

namespace fs = std::filesystem;

namespace trashcli {

static sqlite3* g_db = nullptr;

// ============================================================
// Internal helpers
// ============================================================

static bool path_exists(const fs::path& path) {
    std::error_code ec;
    fs::file_status st = fs::symlink_status(path, ec);

    if (ec) {
        return false;
    }

    return fs::exists(st) || fs::is_symlink(st);
}

static bool path_is_directory(const fs::path& path) {
    std::error_code ec;
    fs::file_status st = fs::symlink_status(path, ec);

    if (ec) {
        return false;
    }

    return fs::is_directory(st);
}

static bool exec_sql(const char* sql) {
    if (!g_db) {
        return false;
    }

    char* err_msg = nullptr;

    int rc = sqlite3_exec(g_db, sql, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Error: "
                  << (err_msg ? err_msg : sqlite3_errmsg(g_db))
                  << "\n";

        if (err_msg) {
            sqlite3_free(err_msg);
        }

        return false;
    }

    return true;
}

static bool column_exists(const std::string& table,
                          const std::string& column) {
    if (!g_db) {
        return false;
    }

    const char* sql = "PRAGMA table_info(";

    std::string query = sql + table + ")";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        query.c_str(),
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {
        return false;
    }

    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* name =
            sqlite3_column_text(stmt, 1);

        if (name &&
            column == reinterpret_cast<const char*>(name)) {
            found = true;
            break;
        }
    }

    sqlite3_finalize(stmt);

    return found;
}

static bool migrate_database_schema() {
    if (!g_db) {
        return false;
    }

    /*
     * Older versions of the project may have:
     *
     * id
     * name
     * original_path
     * trash_path
     * deleted_at
     * size
     * file_type
     *
     * New version requires:
     *
     * trash_id
     * stored_name
     *
     * Add missing columns instead of destroying existing data.
     */

    if (!column_exists("trash_entries", "trash_id")) {
        if (!exec_sql(
                "ALTER TABLE trash_entries "
                "ADD COLUMN trash_id TEXT")) {
            return false;
        }
    }

    if (!column_exists("trash_entries", "stored_name")) {
        if (!exec_sql(
                "ALTER TABLE trash_entries "
                "ADD COLUMN stored_name TEXT")) {
            return false;
        }
    }

    /*
     * Backfill trash_id and stored_name for entries created
     * by an older version.
     */
    {
        const char* sql =
            "SELECT id, name, trash_path, trash_id, stored_name "
            "FROM trash_entries";

        sqlite3_stmt* stmt = nullptr;

        int rc = sqlite3_prepare_v2(
            g_db,
            sql,
            -1,
            &stmt,
            nullptr
        );

        if (rc != SQLITE_OK) {
            return false;
        }

        struct RowUpdate {
            sqlite3_int64 id;
            std::string name;
            std::string trash_path;
            std::string trash_id;
            std::string stored_name;
        };

        std::vector<RowUpdate> updates;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            RowUpdate row{};

            row.id = sqlite3_column_int64(stmt, 0);

            const unsigned char* name =
                sqlite3_column_text(stmt, 1);

            const unsigned char* trash_path =
                sqlite3_column_text(stmt, 2);

            const unsigned char* trash_id =
                sqlite3_column_text(stmt, 3);

            const unsigned char* stored_name =
                sqlite3_column_text(stmt, 4);

            if (name) {
                row.name =
                    reinterpret_cast<const char*>(name);
            }

            if (trash_path) {
                row.trash_path =
                    reinterpret_cast<const char*>(trash_path);
            }

            if (trash_id) {
                row.trash_id =
                    reinterpret_cast<const char*>(trash_id);
            }

            if (stored_name) {
                row.stored_name =
                    reinterpret_cast<const char*>(stored_name);
            }

            if (row.trash_id.empty()) {
                row.trash_id =
                    fs::path(row.trash_path).filename().string();
            }

            if (row.stored_name.empty()) {
                row.stored_name = row.name;
            }

            updates.push_back(std::move(row));
        }

        sqlite3_finalize(stmt);

        const char* update_sql =
            "UPDATE trash_entries "
            "SET trash_id = ?, stored_name = ? "
            "WHERE id = ?";

        for (const auto& row : updates) {
            sqlite3_stmt* update_stmt = nullptr;

            rc = sqlite3_prepare_v2(
                g_db,
                update_sql,
                -1,
                &update_stmt,
                nullptr
            );

            if (rc != SQLITE_OK) {
                return false;
            }

            sqlite3_bind_text(
                update_stmt,
                1,
                row.trash_id.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                update_stmt,
                2,
                row.stored_name.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_int64(
                update_stmt,
                3,
                row.id
            );

            rc = sqlite3_step(update_stmt);

            sqlite3_finalize(update_stmt);

            if (rc != SQLITE_DONE) {
                return false;
            }
        }
    }

    /*
     * Index creation is intentionally done after migration.
     */
    exec_sql(
        "CREATE INDEX IF NOT EXISTS "
        "idx_original_path "
        "ON trash_entries(original_path)"
    );

    exec_sql(
        "CREATE INDEX IF NOT EXISTS "
        "idx_deleted_at "
        "ON trash_entries(deleted_at)"
    );

    exec_sql(
        "CREATE INDEX IF NOT EXISTS "
        "idx_trash_id "
        "ON trash_entries(trash_id)"
    );

    exec_sql(
        "CREATE INDEX IF NOT EXISTS "
        "idx_session_dir "
        "ON trash_sessions(session_dir)"
    );

    return true;
}

static bool trash_id_exists_in_database(
    const std::string& trash_id) {

    if (!init_database()) {
        return false;
    }

    const char* sql =
        "SELECT 1 "
        "FROM trash_entries "
        "WHERE trash_id = ? "
        "LIMIT 1";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        trash_id.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    return exists;
}

static std::string generate_unique_trash_id() {

    const std::string trash_base = get_trash_dir();

    static std::random_device rd;
    static std::mt19937_64 generator(rd());

    std::uniform_int_distribution<std::uint64_t> distribution;

    for (int attempt = 0; attempt < 100; ++attempt) {

        auto now = std::chrono::system_clock::now();

        auto time =
            std::chrono::system_clock::to_time_t(now);

        struct tm tm_buf{};
        localtime_r(&time, &tm_buf);

        char time_buf[32]{};

        std::strftime(
            time_buf,
            sizeof(time_buf),
            "%Y%m%d-%H%M%S",
            &tm_buf
        );

        std::uint64_t random_value =
            distribution(generator);

        std::ostringstream oss;

        oss << time_buf
            << "-"
            << std::hex
            << std::setw(12)
            << std::setfill('0')
            << (random_value & 0xFFFFFFFFFFFFULL);

        std::string candidate = oss.str();

        fs::path directory =
            fs::path(trash_base) / candidate;

        if (path_exists(directory)) {
            continue;
        }

        if (trash_id_exists_in_database(candidate)) {
            continue;
        }

        return candidate;
    }

    /*
     * Practically unreachable fallback.
     */
    auto now =
        std::chrono::high_resolution_clock::now()
            .time_since_epoch()
            .count();

    return "trash-" + std::to_string(now);
}

static bool ensure_parent_directory(
    const fs::path& path) {

    fs::path parent = path.parent_path();

    if (parent.empty()) {
        return true;
    }

    std::error_code ec;

    fs::create_directories(parent, ec);

    if (ec) {
        std::cerr
            << "Error: Cannot create parent directory: "
            << parent
            << ": "
            << ec.message()
            << "\n";

        return false;
    }

    return true;
}

static bool move_to_trash(
    const fs::path& src,
    const fs::path& dest,
    std::error_code& ec) {

    ec.clear();

    /*
     * Fast path: same filesystem.
     */
    fs::rename(src, dest, ec);

    if (!ec) {
        return true;
    }

    /*
     * Cross-filesystem fallback.
     *
     * rename() commonly returns EXDEV in this case.
     */
    if (ec.value() != EXDEV) {
        return false;
    }

    std::error_code copy_ec;

    fs::copy(
        src,
        dest,
        fs::copy_options::recursive,
        copy_ec
    );

    if (copy_ec) {
        ec = copy_ec;
        return false;
    }

    /*
     * Verify destination exists before deleting source.
     */
    if (!path_exists(dest)) {
        ec = std::make_error_code(
            std::errc::io_error
        );

        return false;
    }

    std::error_code remove_ec;

    fs::remove_all(src, remove_ec);

    if (remove_ec) {
        /*
         * Try to clean up the copied destination because
         * original removal failed.
         */
        std::error_code cleanup_ec;

        fs::remove_all(dest, cleanup_ec);

        ec = remove_ec;
        return false;
    }

    return true;
}

static bool is_path_inside(
    const fs::path& base,
    const fs::path& child) {

    std::error_code ec1;
    std::error_code ec2;

    fs::path normalized_base =
        fs::weakly_canonical(base, ec1);

    fs::path normalized_child =
        fs::weakly_canonical(child, ec2);

    if (ec1 || ec2) {
        /*
         * Fall back to lexical comparison when canonicalization
         * is unavailable.
         */
        normalized_base = base.lexically_normal();
        normalized_child = child.lexically_normal();
    }

    auto base_it = normalized_base.begin();
    auto child_it = normalized_child.begin();

    for (; base_it != normalized_base.end();
         ++base_it, ++child_it) {

        if (child_it == normalized_child.end()) {
            return false;
        }

        if (*base_it != *child_it) {
            return false;
        }
    }

    return true;
}

static bool remove_directory_entry(
    const fs::path& path) {

    if (!path_exists(path)) {
        return true;
    }

    std::error_code ec;

    fs::remove_all(path, ec);

    if (ec) {
        std::cerr
            << "Error: Cannot remove "
            << path
            << ": "
            << ec.message()
            << "\n";

        return false;
    }

    return true;
}

// ============================================================
// Public API
// ============================================================

bool init_database() {

    if (g_db) {
        return true;
    }

    std::string trash_base = get_trash_dir();

    if (trash_base.empty()) {
        std::cerr
            << "Error: Cannot determine trash directory\n";
        return false;
    }

    std::error_code ec;

    fs::create_directories(
        trash_base,
        ec
    );

    if (ec) {
        std::cerr
            << "Error: Cannot create trash directory: "
            << ec.message()
            << "\n";

        return false;
    }

    std::string db_path =
        trash_base + "/trash.db";

    int rc = sqlite3_open(
        db_path.c_str(),
        &g_db
    );

    if (rc != SQLITE_OK) {

        std::cerr
            << "Error: Cannot open database: "
            << (g_db
                    ? sqlite3_errmsg(g_db)
                    : "unknown error")
            << "\n";

        if (g_db) {
            sqlite3_close(g_db);
            g_db = nullptr;
        }

        return false;
    }

    /*
     * SQLite configuration.
     */
    if (!exec_sql("PRAGMA foreign_keys = ON;")) {
        close_database();
        return false;
    }

    exec_sql("PRAGMA journal_mode=WAL;");
    exec_sql("PRAGMA synchronous=NORMAL;");

    /*
     * Current schema.
     */
    const char* sql = R"SQL(
        CREATE TABLE IF NOT EXISTS trash_entries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            trash_id TEXT,
            name TEXT NOT NULL,
            original_path TEXT NOT NULL,
            trash_path TEXT NOT NULL,
            stored_name TEXT,
            deleted_at TEXT DEFAULT CURRENT_TIMESTAMP,
            size INTEGER DEFAULT 0,
            file_type TEXT DEFAULT 'file'
        );

        CREATE TABLE IF NOT EXISTS trash_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_dir TEXT NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )SQL";

    if (!exec_sql(sql)) {
        close_database();
        return false;
    }

    /*
     * Upgrade databases created by older versions.
     */
    if (!migrate_database_schema()) {
        std::cerr
            << "Error: Database migration failed\n";

        close_database();
        return false;
    }

    return true;
}

void close_database() {

    if (g_db) {

        sqlite3_close(g_db);
        g_db = nullptr;
    }
}

std::string get_trash_dir() {

    const char* xdg_data_home =
        std::getenv("XDG_DATA_HOME");

    if (xdg_data_home &&
        xdg_data_home[0] != '\0') {

        return std::string(xdg_data_home)
            + "/trash";
    }

    const char* home =
        std::getenv("HOME");

    if (!home || home[0] == '\0') {
        return "";
    }

    return std::string(home)
        + "/.local/share/trash";
}

std::string generate_trash_subdir() {

    auto now =
        std::chrono::system_clock::now();

    auto time =
        std::chrono::system_clock::to_time_t(now);

    struct tm tm_buf{};

    localtime_r(
        &time,
        &tm_buf
    );

    char time_buf[32]{};

    std::strftime(
        time_buf,
        sizeof(time_buf),
        "%Y%m%d-%H%M%S",
        &tm_buf
    );

    static std::random_device rd;
    static std::mt19937_64 generator(rd());

    std::uint64_t value =
        generator();

    std::ostringstream oss;

    oss << time_buf
        << "-"
        << std::hex
        << std::setw(12)
        << std::setfill('0')
        << (value & 0xFFFFFFFFFFFFULL);

    return oss.str();
}

std::string get_current_time_str() {

    auto now =
        std::chrono::system_clock::now();

    auto time =
        std::chrono::system_clock::to_time_t(now);

    struct tm tm_buf{};

    localtime_r(
        &time,
        &tm_buf
    );

    char time_str[64]{};

    std::strftime(
        time_str,
        sizeof(time_str),
        "%Y-%m-%d %H:%M:%S",
        &tm_buf
    );

    return std::string(time_str);
}

std::size_t calculate_directory_size(
    const fs::path& path) {

    std::uintmax_t total = 0;

    std::error_code ec;

    fs::recursive_directory_iterator it(
        path,
        fs::directory_options::skip_permission_denied,
        ec
    );

    fs::recursive_directory_iterator end;

    while (!ec && it != end) {

        std::error_code status_ec;

        if (it->is_regular_file(status_ec) &&
            !status_ec) {

            std::error_code size_ec;

            std::uintmax_t size =
                it->file_size(size_ec);

            if (!size_ec) {
                total += size;
            }
        }

        it.increment(ec);
    }

    return static_cast<std::size_t>(total);
}

bool insert_trash_entry(
    const std::string& trash_id,
    const std::string& name,
    const std::string& original_path,
    const std::string& trash_path,
    const std::string& stored_name,
    std::size_t size,
    const std::string& type) {

    if (!init_database()) {
        return false;
    }

    const char* sql =
        "INSERT INTO trash_entries "
        "(trash_id, name, original_path, trash_path, "
        " stored_name, deleted_at, size, file_type) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {

        std::cerr
            << "Error: "
            << sqlite3_errmsg(g_db)
            << "\n";

        return false;
    }

    const std::string deleted_at =
        get_current_time_str();

    sqlite3_bind_text(
        stmt,
        1,
        trash_id.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        2,
        name.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        3,
        original_path.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        4,
        trash_path.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        5,
        stored_name.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        6,
        deleted_at.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_int64(
        stmt,
        7,
        static_cast<sqlite3_int64>(size)
    );

    sqlite3_bind_text(
        stmt,
        8,
        type.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    rc = sqlite3_step(stmt);

    bool success = (rc == SQLITE_DONE);

    if (!success) {
        std::cerr
            << "Error: Database insert failed: "
            << sqlite3_errmsg(g_db)
            << "\n";
    }

    sqlite3_finalize(stmt);

    return success;
}

bool delete_trash_entry(
    const std::string& trash_id) {

    if (!init_database()) {
        return false;
    }

    const char* sql =
        "DELETE FROM trash_entries "
        "WHERE trash_id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        trash_id.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    rc = sqlite3_step(stmt);

    int changed =
        sqlite3_changes(g_db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return false;
    }

    return changed == 1;
}

// ============================================================
// Trash a file
// ============================================================

bool trash_file(
    const std::string& filepath) {

    fs::path fp(filepath);

    /*
     * Keep file and directory operations separate.
     */
    if (!path_exists(fp)) {

        std::cerr
            << "Error: File not found: "
            << filepath
            << "\n";

        return false;
    }

    if (path_is_directory(fp)) {

        std::cerr
            << "Error: Path is a directory: "
            << filepath
            << "\n";

        std::cerr
            << "Use recursive mode (-r/-rf) for directories.\n";

        return false;
    }

    std::string trash_base =
        get_trash_dir();

    if (trash_base.empty()) {
        return false;
    }

    struct stat st{};

    if (stat(
            filepath.c_str(),
            &st) != 0) {

        /*
         * A symlink or unusual filesystem object may fail stat().
         * Still try to use lstat() as fallback.
         */
        if (lstat(
                filepath.c_str(),
                &st) != 0) {

            std::cerr
                << "Error: Cannot stat file: "
                << filepath
                << ": "
                << std::strerror(errno)
                << "\n";

            return false;
        }
    }

    fs::path abs_fp;

    try {
        abs_fp =
            fs::absolute(fp);
    } catch (const fs::filesystem_error& e) {

        std::cerr
            << "Error: Cannot resolve path: "
            << e.what()
            << "\n";

        return false;
    }

    const std::string abs_path =
        abs_fp.lexically_normal().string();

    const std::string trash_id =
        generate_unique_trash_id();

    fs::path trash_dir =
        fs::path(trash_base) / trash_id;

    fs::path data_dir =
        trash_dir / "data";

    std::error_code ec;

    fs::create_directories(
        data_dir,
        ec
    );

    if (ec) {

        std::cerr
            << "Error: Cannot create trash directory: "
            << ec.message()
            << "\n";

        return false;
    }

    /*
     * Original filename.
     */
    std::string original_name =
        fp.filename().string();

    if (original_name.empty()) {

        std::cerr
            << "Error: Invalid filename: "
            << filepath
            << "\n";

        remove_directory_entry(trash_dir);

        return false;
    }

    /*
     * Resolve filename collision inside the trash entry.
     */
    std::string stored_name =
        original_name;

    fs::path dest =
        data_dir / stored_name;

    int counter = 1;

    while (path_exists(dest)) {

        std::string stem =
            fp.stem().string();

        std::string ext =
            fp.extension().string();

        stored_name =
            stem
            + "_"
            + std::to_string(counter)
            + ext;

        dest =
            data_dir / stored_name;

        ++counter;
    }

    /*
     * Move original into trash.
     */
    if (!move_to_trash(
            fp,
            dest,
            ec)) {

        std::cerr
            << "Error: Cannot move to trash: "
            << ec.message()
            << "\n";

        remove_directory_entry(trash_dir);

        return false;
    }

    /*
     * IMPORTANT:
     *
     * Do not use fp after rename as if it still refers to the
     * original filesystem object.
     */

    const std::string stored_path =
        trash_dir.string();

    const std::string type =
        "file";

    /*
     * Insert metadata.
     */
    if (!insert_trash_entry(
            trash_id,
            original_name,
            abs_path,
            stored_path,
            stored_name,
            static_cast<std::size_t>(st.st_size),
            type)) {

        /*
         * Database failed.
         * Roll filesystem operation back.
         */
        std::error_code rollback_ec;

        move_to_trash(
            dest,
            abs_fp,
            rollback_ec
        );

        if (rollback_ec) {

            std::cerr
                << "CRITICAL: Database insert failed "
                   "AND rollback failed.\n";

            std::cerr
                << "File is currently stored at: "
                << dest
                << "\n";

            std::cerr
                << "Error: "
                << rollback_ec.message()
                << "\n";

            return false;
        }

        remove_directory_entry(trash_dir);

        std::cerr
            << "Error: Database insert failed. "
               "Original file restored.\n";

        return false;
    }

    return true;
}

// ============================================================
// Trash a directory
// ============================================================

bool trash_recursive(
    const std::string& dirpath) {

    fs::path dp(dirpath);

    if (!path_exists(dp)) {

        std::cerr
            << "Error: Directory not found: "
            << dirpath
            << "\n";

        return false;
    }

    if (!path_is_directory(dp)) {

        std::cerr
            << "Error: Path is not a directory: "
            << dirpath
            << "\n";

        return false;
    }

    std::string trash_base =
        get_trash_dir();

    if (trash_base.empty()) {
        return false;
    }

    struct stat st{};

    if (stat(
            dirpath.c_str(),
            &st) != 0) {

        if (lstat(
                dirpath.c_str(),
                &st) != 0) {

            std::cerr
                << "Error: Cannot stat directory: "
                << dirpath
                << ": "
                << std::strerror(errno)
                << "\n";

            return false;
        }
    }

    fs::path abs_dp;

    try {
        // First get absolute path, then normalize by removing trailing slashes
        // and using lexically_normal. We need to handle trailing slashes
        // because fs::absolute() preserves them and filename() returns empty.
        abs_dp = fs::absolute(dp);
        
        // Remove trailing slashes from the path string
        std::string path_str = abs_dp.string();
        while (!path_str.empty() && 
               (path_str.back() == '/' || path_str.back() == '\\')) {
            path_str.pop_back();
        }
        
        abs_dp = fs::path(path_str).lexically_normal();
    } catch (const fs::filesystem_error& e) {

        std::cerr
            << "Error: Cannot resolve directory: "
            << e.what()
            << "\n";

        return false;
    }

    const std::string abs_path =
        abs_dp.string();

    // Get the original name from the normalized absolute path,
    // not from the original user input. This handles:
    //   checkout
    //   checkout/
    //   ./checkout/
    // All normalize to the same path with the same filename.
    const std::string original_name =
        abs_dp.filename().string();

    if (original_name.empty()) {

        std::cerr
            << "Error: Invalid directory name\n";

        return false;
    }

    const std::string trash_id =
        generate_unique_trash_id();

    fs::path trash_dir =
        fs::path(trash_base) / trash_id;

    fs::path data_dir =
        trash_dir / "data";

    std::error_code ec;

    fs::create_directories(
        data_dir,
        ec
    );

    if (ec) {

        std::cerr
            << "Error: Cannot create trash directory: "
            << ec.message()
            << "\n";

        return false;
    }

    std::string stored_name =
        original_name;

    fs::path dest =
        data_dir / stored_name;

    int counter = 1;

    while (path_exists(dest)) {

        stored_name =
            original_name
            + "_"
            + std::to_string(counter);

        dest =
            data_dir / stored_name;

        ++counter;
    }

    /*
     * Move the entire directory tree.
     *
     * There is no need to manually recurse:
     * rename() moves the entire directory tree atomically when
     * source and destination are on the same filesystem.
     */
    if (!move_to_trash(
            abs_dp,  // Use normalized absolute path
            dest,
            ec)) {

        std::cerr
            << "Error: Cannot move directory to trash: "
            << ec.message()
            << "\n";

        remove_directory_entry(trash_dir);

        return false;
    }

    /*
     * Calculate the actual size AFTER moving.
     */
    std::size_t dir_size =
        calculate_directory_size(dest);

    const std::string stored_path =
        trash_dir.string();

    if (!insert_trash_entry(
            trash_id,
            original_name,
            abs_path,
            stored_path,
            stored_name,
            dir_size,
            "directory")) {

        /*
         * DB failed.
         * Try to put directory back exactly where it came from.
         */
        std::error_code rollback_ec;

        move_to_trash(
            dest,
            abs_dp,
            rollback_ec
        );

        if (rollback_ec) {

            std::cerr
                << "CRITICAL: Database insert failed "
                   "AND directory rollback failed.\n";

            std::cerr
                << "Directory is currently stored at: "
                << dest
                << "\n";

            std::cerr
                << "Error: "
                << rollback_ec.message()
                << "\n";

            return false;
        }

        remove_directory_entry(trash_dir);

        std::cerr
            << "Error: Database insert failed. "
               "Original directory restored.\n";

        return false;
    }

    return true;
}

// ============================================================
// Restore
// ============================================================

bool restore_file(
    const std::string& trash_id,
    bool force) {

    if (trash_id.empty()) {

        std::cerr
            << "Error: Empty trash ID\n";

        return false;
    }

    if (!init_database()) {
        return false;
    }

    /*
     * Never construct filesystem paths directly from the user's
     * trash_id before validating that the ID actually exists in
     * SQLite.
     */
    const char* sql =
        "SELECT "
        "original_path, "
        "file_type, "
        "name, "
        "stored_name, "
        "trash_path "
        "FROM trash_entries "
        "WHERE trash_id = ? "
        "LIMIT 1";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {

        std::cerr
            << "Error: Cannot query trash metadata: "
            << sqlite3_errmsg(g_db)
            << "\n";

        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        trash_id.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    std::string original;
    std::string file_type;
    std::string entry_name;
    std::string stored_name;
    std::string trash_path;

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {

        const unsigned char* value;

        value = sqlite3_column_text(stmt, 0);
        if (value) {
            original =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 1);
        if (value) {
            file_type =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 2);
        if (value) {
            entry_name =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 3);
        if (value) {
            stored_name =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 4);
        if (value) {
            trash_path =
                reinterpret_cast<const char*>(value);
        }
    }

    sqlite3_finalize(stmt);

    if (original.empty() ||
        stored_name.empty() ||
        trash_path.empty()) {

        std::cerr
            << "Error: Trash metadata not found: "
            << trash_id
            << "\n";

        return false;
    }

    /*
     * Verify that the metadata path belongs to our trash root.
     */
    fs::path trash_base =
        get_trash_dir();

    fs::path trash_dir(trash_path);

    if (!is_path_inside(
            trash_base,
            trash_dir)) {

        std::cerr
            << "Error: Invalid trash path in metadata\n";

        return false;
    }

    /*
     * Also ensure the final directory name is actually the
     * requested ID.
     */
    if (trash_dir.filename() != trash_id) {

        std::cerr
            << "Error: Trash metadata ID/path mismatch\n";

        return false;
    }

    fs::path data_dir =
        trash_dir / "data";

    fs::path source_path =
        data_dir / stored_name;

    fs::path original_path(original);

    /*
     * Verify exact source.
     *
     * NO "take the first file" fallback.
     */
    if (!path_exists(trash_dir)) {

        std::cerr
            << "Error: Trash data is missing for ID: "
            << trash_id
            << "\n";

        return false;
    }

    if (!path_exists(data_dir)) {

        std::cerr
            << "Error: Trash data directory is missing: "
            << data_dir
            << "\n";

        return false;
    }

    if (!path_exists(source_path)) {

        std::cerr
            << "Error: Stored object is missing: "
            << source_path
            << "\n";

        return false;
    }

    /*
     * Check destination collision.
     */
    if (path_exists(original_path) && !force) {

        std::cerr
            << "Error: Already exists at: "
            << original_path
            << "\n";

        std::cerr
            << "Use -ref to overwrite.\n";

        return false;
    }

    /*
     * Ensure destination parent exists.
     */
    if (!ensure_parent_directory(original_path)) {
        return false;
    }

    /*
     * Force overwrite.
     */
    if (path_exists(original_path) && force) {

        std::error_code remove_ec;

        fs::remove_all(
            original_path,
            remove_ec
        );

        if (remove_ec) {

            std::cerr
                << "Error: Cannot remove existing destination: "
                << remove_ec.message()
                << "\n";

            return false;
        }
    }

    /*
     * Restore.
     *
     * move_to_trash() is intentionally generic enough to also
     * handle cross-filesystem restoration.
     */
    std::error_code move_ec;

    if (!move_to_trash(
            source_path,
            original_path,
            move_ec)) {

        std::cerr
            << "Error: Cannot restore "
            << entry_name
            << ": "
            << move_ec.message()
            << "\n";

        return false;
    }

    /*
     * Metadata must be deleted only after the actual restore
     * succeeds.
     */
    if (!delete_trash_entry(trash_id)) {

        /*
         * DB delete failed.
         *
         * Try rollback so the object remains represented by
         * the metadata entry.
         */
        std::error_code rollback_ec;

        move_to_trash(
            original_path,
            source_path,
            rollback_ec
        );

        if (rollback_ec) {

            std::cerr
                << "CRITICAL: Restore succeeded, but database "
                   "cleanup failed AND rollback failed.\n";

            std::cerr
                << "Restored object is at: "
                << original_path
                << "\n";

            std::cerr
                << "Trash metadata still refers to: "
                << source_path
                << "\n";

            return false;
        }

        std::cerr
            << "Error: Database cleanup failed. "
               "Restore operation rolled back.\n";

        return false;
    }

    /*
     * Metadata is gone, so the trash directory can now be removed.
     */
    if (!remove_directory_entry(trash_dir)) {

        std::cerr
            << "Warning: Restored successfully, but could not "
               "remove trash data: "
            << trash_dir
            << "\n";

        /*
         * Not a restore failure anymore.
         */
    }

    return true;
}

// ============================================================
// Permanent delete
// ============================================================

bool permanent_delete(
    const std::string& filepath) {

    fs::path path(filepath);

    if (!path_exists(path)) {

        std::cerr
            << "Error: File not found: "
            << filepath
            << "\n";

        return false;
    }

    std::error_code ec;

    fs::remove_all(
        path,
        ec
    );

    if (ec) {

        std::cerr
            << "Error: Cannot permanently delete "
            << filepath
            << ": "
            << ec.message()
            << "\n";

        return false;
    }

    return true;
}

// ============================================================
// Empty trash
// ============================================================

bool empty_trash(
    bool keep_root) {

    (void)keep_root;

    if (!init_database()) {
        return false;
    }

    const std::string trash_base_str =
        get_trash_dir();

    if (trash_base_str.empty()) {
        return false;
    }

    fs::path trash_base(
        trash_base_str
    );

    if (!path_exists(trash_base)) {
        return true;
    }

    /*
     * First process entries known by SQLite.
     *
     * We deliberately delete each DB row only after its
     * corresponding filesystem object has been removed.
     */
    const char* sql =
        "SELECT trash_id, trash_path "
        "FROM trash_entries "
        "ORDER BY deleted_at ASC";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {

        std::cerr
            << "Error: Cannot read trash database: "
            << sqlite3_errmsg(g_db)
            << "\n";

        return false;
    }

    struct TrashPath {
        std::string id;
        std::string path;
    };

    std::vector<TrashPath> entries;

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        const unsigned char* id =
            sqlite3_column_text(stmt, 0);

        const unsigned char* path =
            sqlite3_column_text(stmt, 1);

        if (!id || !path) {
            continue;
        }

        entries.push_back({
            reinterpret_cast<const char*>(id),
            reinterpret_cast<const char*>(path)
        });
    }

    sqlite3_finalize(stmt);

    bool all_success = true;

    for (const auto& entry : entries) {

        fs::path entry_path(entry.path);

        /*
         * Never allow database corruption to cause us to delete
         * something outside the trash root.
         */
        if (!is_path_inside(
                trash_base,
                entry_path)) {

            std::cerr
                << "Warning: Skipping unsafe trash path: "
                << entry_path
                << "\n";

            all_success = false;
            continue;
        }

        if (!remove_directory_entry(entry_path)) {

            all_success = false;
            continue;
        }

        if (!delete_trash_entry(entry.id)) {

            std::cerr
                << "Warning: Deleted trash data but could not "
                   "delete database entry: "
                << entry.id
                << "\n";

            all_success = false;
        }
    }

    /*
     * Cleanup orphan directories that have no DB entry.
     *
     * trash.db itself is NEVER touched.
     */
    std::error_code iter_ec;

    for (fs::directory_iterator it(
             trash_base,
             fs::directory_options::skip_permission_denied,
             iter_ec);
         !iter_ec && it != fs::directory_iterator();
         it.increment(iter_ec)) {

        const fs::path path =
            it->path();

        if (!it->is_directory()) {
            continue;
        }

        /*
         * Any directory directly under trash root is a trash
         * entry directory.
         */
        if (!remove_directory_entry(path)) {
            all_success = false;
        }
    }

    if (iter_ec) {

        std::cerr
            << "Warning: Cannot fully scan trash directory: "
            << iter_ec.message()
            << "\n";

        all_success = false;
    }

    /*
     * Clear session metadata after trash has been processed.
     */
    exec_sql("DELETE FROM trash_sessions");

    return all_success;
}

// ============================================================
// List trash
// ============================================================

std::vector<TrashEntry> list_trash() {

    std::vector<TrashEntry> entries;

    if (!init_database()) {
        return entries;
    }

    const char* sql =
        "SELECT "
        "name, "
        "original_path, "
        "trash_path, "
        "deleted_at, "
        "size, "
        "file_type "
        "FROM trash_entries "
        "ORDER BY deleted_at DESC";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        g_db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK) {
        return entries;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        TrashEntry te{};

        const unsigned char* value;

        value = sqlite3_column_text(stmt, 0);
        if (value) {
            te.name =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 1);
        if (value) {
            te.original_path =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 2);
        if (value) {
            te.trash_path =
                reinterpret_cast<const char*>(value);
        }

        value = sqlite3_column_text(stmt, 3);
        if (value) {
            te.deleted_at =
                reinterpret_cast<const char*>(value);
        }

        te.size =
            static_cast<std::size_t>(
                sqlite3_column_int64(stmt, 4)
            );

        value = sqlite3_column_text(stmt, 5);
        if (value) {
            te.type =
                reinterpret_cast<const char*>(value);
        }

        entries.push_back(
            std::move(te)
        );
    }

    sqlite3_finalize(stmt);

    return entries;
}

// ============================================================
// Show info
// ============================================================

bool show_info(
    const std::string& filepath,
    bool detailed) {

    fs::path fp(filepath);

    if (!path_exists(fp)) {

        std::cerr
            << "Error: File not found: "
            << filepath
            << "\n";

        return false;
    }

    struct stat st{};

    if (lstat(
            filepath.c_str(),
            &st) != 0) {

        std::cerr
            << "Error: Cannot stat path: "
            << filepath
            << ": "
            << std::strerror(errno)
            << "\n";

        return false;
    }

    std::cout
        << "Name: "
        << fp.filename().string()
        << "\n";

    std::cout
        << "Path: "
        << filepath
        << "\n";

    /*
     * Directory size should represent its contents rather than
     * the directory inode's st_size.
     */
    std::size_t logical_size =
        static_cast<std::size_t>(
            st.st_size
        );

    if (S_ISDIR(st.st_mode)) {

        logical_size =
            calculate_directory_size(fp);
    }

    if (detailed) {

        std::cout
            << "Size: "
            << logical_size
            << " bytes\n";

        std::cout
            << "Permissions: ";

        std::cout
            << ((st.st_mode & S_IRUSR)
                    ? "r"
                    : "-");

        std::cout
            << ((st.st_mode & S_IWUSR)
                    ? "w"
                    : "-");

        std::cout
            << ((st.st_mode & S_IXUSR)
                    ? "x"
                    : "-");

        std::cout
            << ((st.st_mode & S_IRGRP)
                    ? "r"
                    : "-");

        std::cout
            << ((st.st_mode & S_IWGRP)
                    ? "w"
                    : "-");

        std::cout
            << ((st.st_mode & S_IXGRP)
                    ? "x"
                    : "-");

        std::cout
            << ((st.st_mode & S_IROTH)
                    ? "r"
                    : "-");

        std::cout
            << ((st.st_mode & S_IWOTH)
                    ? "w"
                    : "-");

        std::cout
            << ((st.st_mode & S_IXOTH)
                    ? "x"
                    : "-");

        std::cout << "\n";

    } else {

        double size =
            static_cast<double>(
                logical_size
            );

        const char* units[] = {
            "B",
            "KB",
            "MB",
            "GB",
            "TB"
        };

        int unit = 0;

        while (size >= 1024.0 &&
               unit < 4) {

            size /= 1024.0;
            ++unit;
        }

        std::cout
            << "Size: "
            << std::fixed
            << std::setprecision(1)
            << size
            << " "
            << units[unit]
            << "\n";
    }

    struct tm tm_buf{};

    localtime_r(
        &st.st_mtime,
        &tm_buf
    );

    char time_str[64]{};

    std::strftime(
        time_str,
        sizeof(time_str),
        "%Y-%m-%d %H:%M:%S",
        &tm_buf
    );

    std::cout
        << "Modified: "
        << time_str
        << "\n";

    return true;
}

} // namespace trashcli
