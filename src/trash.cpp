#include "trashcli.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <sqlite3.h>

namespace fs = std::filesystem;

namespace trashcli {

static sqlite3* g_db = nullptr;

bool init_database() {
    if (g_db) return true;
    
    std::string trash_base = get_trash_dir();
    if (trash_base.empty()) return false;
    
    fs::create_directories(trash_base);
    std::string db_path = trash_base + "/trash.db";
    
    int rc = sqlite3_open(db_path.c_str(), &g_db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(g_db) << "\n";
        return false;
    }
    
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS trash_entries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            original_path TEXT NOT NULL,
            trash_path TEXT NOT NULL,
            deleted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            size INTEGER DEFAULT 0,
            file_type TEXT DEFAULT 'file'
        );
        
        CREATE TABLE IF NOT EXISTS trash_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_dir TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        
        CREATE INDEX IF NOT EXISTS idx_original_path ON trash_entries(original_path);
        CREATE INDEX IF NOT EXISTS idx_deleted_at ON trash_entries(deleted_at);
        CREATE INDEX IF NOT EXISTS idx_session_dir ON trash_sessions(session_dir);
    )";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(g_db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Cannot create tables: " << errMsg << "\n";
        sqlite3_free(errMsg);
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
    const char* home = getenv("HOME");
    if (!home) return "";
    return std::string(home) + "/.local/share/trash";
}

std::string generate_trash_subdir() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    struct tm tm_buf;
    localtime_r(&time, &tm_buf);

    char time_buf[32];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d-%H%M%S", &tm_buf);

    auto seed = static_cast<unsigned>(time) ^ static_cast<unsigned>(ms.count());
    char hex[6];
    snprintf(hex, sizeof(hex), "%05x", seed & 0xFFFFF);

    return std::string(time_buf) + "-" + hex;
}

std::string get_current_time_str() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    localtime_r(&time, &tm_buf);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return std::string(time_str);
}

bool insert_trash_entry(const std::string& name, const std::string& original_path, 
                        const std::string& trash_path, std::size_t size, const std::string& type) {
    if (!init_database()) return false;
    
    const char* sql = "INSERT INTO trash_entries (name, original_path, trash_path, deleted_at, size, file_type) VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(g_db) << "\n";
        return false;
    }
    
    std::string deleted_at = get_current_time_str();
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, original_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, trash_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, deleted_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 5, static_cast<sqlite3_int64>(size));
    sqlite3_bind_text(stmt, 6, type.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool delete_trash_entry(const std::string& trash_id) {
    if (!init_database()) return false;
    
    const char* sql = "DELETE FROM trash_entries WHERE trash_path LIKE ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;
    
    std::string pattern = "%" + trash_id + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool trash_file(const std::string& filepath) {
    fs::path fp(filepath);
    if (!fs::exists(fp)) {
        std::cerr << "Error: File not found: " << filepath << "\n";
        return false;
    }

    std::string trash_base = get_trash_dir();
    if (trash_base.empty()) return false;

    // Get size and absolute path BEFORE moving
    struct stat st;
    stat(filepath.c_str(), &st);
    std::string abs_path = fs::absolute(fp).string();
    std::string type = fs::is_directory(fp) ? "directory" : "file";

    std::string subdir = trash_base + "/" + generate_trash_subdir() + "/data";
    fs::create_directories(subdir);

    fs::path dest = fs::path(subdir) / fp.filename();

    int counter = 1;
    while (fs::exists(dest)) {
        std::string stem = fp.stem().string();
        std::string ext = fp.extension().string();
        dest = fs::path(subdir) / (stem + "_" + std::to_string(counter) + ext);
        counter++;
    }

    fs::rename(fp, dest);

    std::string trash_dir = fs::path(subdir).parent_path().string();
    
    return insert_trash_entry(fp.filename().string(), abs_path, trash_dir, st.st_size, type);
}

bool trash_recursive(const std::string& dirpath) {
    fs::path dp(dirpath);
    if (!fs::exists(dp) || !fs::is_directory(dp)) {
        std::cerr << "Error: Directory not found: " << dirpath << "\n";
        return false;
    }

    std::string trash_base = get_trash_dir();
    if (trash_base.empty()) return false;

    // Get size and absolute path BEFORE moving
    struct stat st;
    stat(dirpath.c_str(), &st);
    std::string abs_path = fs::absolute(dp).string();
    std::string type = "directory";

    std::string subdir = trash_base + "/" + generate_trash_subdir() + "/data";
    fs::create_directories(subdir);

    fs::path dest = fs::path(subdir) / dp.filename();
    fs::rename(dp, dest);

    std::string trash_dir = fs::path(subdir).parent_path().string();
    
    return insert_trash_entry(dp.filename().string(), abs_path, trash_dir, st.st_size, type);
}

bool restore_file(const std::string& trash_id, bool force) {
    std::string trash_base = get_trash_dir();
    std::string trash_dir = trash_base + "/" + trash_id;
    std::string data_dir = trash_dir + "/data";

    if (!fs::exists(trash_dir)) {
        std::cerr << "Error: Trash entry not found: " << trash_id << "\n";
        return false;
    }

    if (!init_database()) return false;

    const char* sql = "SELECT original_path, file_type FROM trash_entries WHERE trash_path LIKE ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;
    
    std::string pattern = "%" + trash_id + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    
    std::string original;
    std::string file_type;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        original = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        file_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    if (original.empty()) {
        std::cerr << "Error: Metadata not found\n";
        return false;
    }

    if (fs::exists(original) && !force) {
        std::cerr << "Error: File already exists at: " << original << "\n";
        std::cerr << "Use -ref to overwrite.\n";
        return false;
    }

    if (!fs::exists(data_dir)) {
        std::cerr << "Error: Data not found in trash\n";
        return false;
    }

    fs::create_directories(fs::path(original).parent_path());
    
    if (file_type == "directory") {
        // For directory: data_dir contains the directory itself (e.g., data/mydir/)
        // Find the single subdirectory in data_dir and rename it to original
        for (auto& entry : fs::directory_iterator(data_dir)) {
            if (fs::is_directory(entry)) {
                if (fs::exists(original)) {
                    if (force) fs::remove_all(original);
                    else {
                        std::cerr << "Error: Directory already exists\n";
                        return false;
                    }
                }
                fs::rename(entry.path(), original);
                break;
            }
        }
    } else {
        // For file: data_dir contains the file directly
        for (auto& entry : fs::directory_iterator(data_dir)) {
            fs::rename(entry.path(), original);
        }
    }

    delete_trash_entry(trash_id);
    fs::remove_all(trash_dir);
    return true;
}

bool permanent_delete(const std::string& filepath) {
    if (!fs::exists(filepath)) {
        std::cerr << "Error: File not found: " << filepath << "\n";
        return false;
    }
    return fs::remove_all(filepath);
}

bool empty_trash(bool keep_root) {
    if (!init_database()) return false;
    
    const char* sql = "DELETE FROM trash_entries";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(g_db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    
    std::string trash_base = get_trash_dir();
    if (!fs::exists(trash_base)) return true;

    for (auto& entry : fs::directory_iterator(trash_base)) {
        if (entry.is_directory()) {
            fs::remove_all(entry.path());
        } else {
            fs::remove(entry.path());
        }
    }
    return true;
}

std::vector<TrashEntry> list_trash() {
    std::vector<TrashEntry> entries;
    
    if (!init_database()) return entries;

    const char* sql = "SELECT name, original_path, trash_path, deleted_at, size, file_type FROM trash_entries ORDER BY deleted_at DESC";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return entries;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TrashEntry te;
        te.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        te.original_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        te.trash_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        te.deleted_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        te.size = static_cast<std::size_t>(sqlite3_column_int64(stmt, 4));
        te.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        entries.push_back(te);
    }
    sqlite3_finalize(stmt);

    return entries;
}

bool show_info(const std::string& filepath, bool detailed) {
    fs::path fp(filepath);
    if (!fs::exists(fp)) {
        std::cerr << "Error: File not found: " << filepath << "\n";
        return false;
    }

    struct stat st;
    stat(filepath.c_str(), &st);

    std::cout << "Name: " << fp.filename().string() << "\n";
    std::cout << "Path: " << filepath << "\n";

    if (detailed) {
        std::cout << "Size: " << st.st_size << " bytes\n";
        std::cout << "Permissions: ";
        std::cout << ((st.st_mode & S_IRUSR) ? "r" : "-");
        std::cout << ((st.st_mode & S_IWUSR) ? "w" : "-");
        std::cout << ((st.st_mode & S_IXUSR) ? "x" : "-");
        std::cout << ((st.st_mode & S_IRGRP) ? "r" : "-");
        std::cout << ((st.st_mode & S_IWGRP) ? "w" : "-");
        std::cout << ((st.st_mode & S_IXGRP) ? "x" : "-");
        std::cout << ((st.st_mode & S_IROTH) ? "r" : "-");
        std::cout << ((st.st_mode & S_IWOTH) ? "w" : "-");
        std::cout << ((st.st_mode & S_IXOTH) ? "x" : "-");
        std::cout << "\n";
    } else {
        double size = st.st_size;
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unit = 0;
        while (size >= 1024 && unit < 3) { size /= 1024; unit++; }
        std::cout << "Size: " << std::fixed << std::setprecision(1) << size << " " << units[unit] << "\n";
    }

    struct tm tm_buf;
    localtime_r(&st.st_mtime, &tm_buf);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);
    std::cout << "Modified: " << time_str << "\n";

    return true;
}

}