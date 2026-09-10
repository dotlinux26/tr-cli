#include "trashcli.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

namespace fs = std::filesystem;

// ============================================================
// ANSI colors
// ============================================================

namespace color {
    constexpr const char* reset  = "\033[0m";
    constexpr const char* green  = "\033[1;32m";
    constexpr const char* cyan   = "\033[1;36m";
    constexpr const char* yellow = "\033[1;33m";
    constexpr const char* gray   = "\033[90m";
    constexpr const char* red    = "\033[1;31m";
    constexpr const char* white  = "\033[1;37m";
}

// ============================================================
// Display helpers
// ============================================================

void print_banner() {
    std::cout << "\n";
    std::cout << color::green;

    std::cout
        << "  ████████╗███████╗██████╗ ███╗   ███╗\n"
        << "  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║\n"
        << "     ██║   █████╗  ██████╔╝██╔████╔██║\n"
        << "     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║\n"
        << "     ██║   ███████╗██║  ██║██║ ╚═╝ ██║\n"
        << "     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝\n";

    std::cout << color::reset;

    std::cout << "\n";

    std::cout
        << "  " << color::cyan
        << "Trash CLI"
        << color::reset
        << " - Công cụ quản lý thùng rác cho Linux "
        << "(Trash management tool for Linux)\n";

    std::cout
        << "  " << color::gray
        << "Phiên bản 1.0.0 (Version) | (c) 2026 Nguyễn Đức Cảnh"
        << color::reset
        << "\n";

    std::cout << "\n";
}

void print_usage() {
    print_banner();

    std::cout
        << "  " << color::yellow
        << "Sử dụng (Usage):"
        << color::reset
        << "\n";

    std::cout
        << "    tr <file>             "
        << "Đưa file vào thùng rác "
        << "(Move file to trash)\n";

    std::cout
        << "    tr -r <thu muc>       "
        << "Đưa thư mục vào thùng rác "
        << "(Move directory to trash)\n";

    std::cout
        << "    tr -f <path>          "
        << "Xóa vĩnh viễn file/thư mục "
        << "(Permanent delete)\n";

    std::cout
        << "    tr -rf <thu muc>      "
        << "Xóa vĩnh viễn thư mục "
        << "(Permanent delete directory)\n";

    std::cout
        << "    tr -rf <thu muc>/*    "
        << "Xóa nội dung, giữ thư mục "
        << "(Delete contents, keep folder)\n";

    std::cout
        << "    tr -rf *              "
        << "Xóa nội dung thư mục hiện tại "
        << "(Delete current directory contents)\n";

    std::cout
        << "    tr -re <ID>           "
        << "Khôi phục theo ID "
        << "(Restore by ID)\n";

    std::cout
        << "    tr -ref <ID>          "
        << "Khôi phục ghi đè theo ID "
        << "(Restore and overwrite by ID)\n";

    std::cout
        << "    tr -l                 "
        << "Liệt kê trash "
        << "(List trash)\n";

    std::cout
        << "    tr -i <ID>            "
        << "Xem thông tin theo ID "
        << "(View info by ID)\n";

    std::cout
        << "    tr -info <ID>         "
        << "Xem thông tin chi tiết "
        << "(View detailed info)\n";

    std::cout
        << "    tr -e                 "
        << "Xóa toàn bộ trash "
        << "(Empty trash)\n";

    std::cout
        << "    tr -h, --help         "
        << "Hiển thị hướng dẫn "
        << "(Show help)\n";

    std::cout
        << "    tr -v, --version      "
        << "Hiển thị phiên bản "
        << "(Show version)\n";

    std::cout << "\n";

    std::cout
        << "  " << color::yellow
        << "Ví dụ (Example):"
        << color::reset
        << "\n";

    std::cout
        << "    tr test.txt           "
        << "Đưa test.txt vào trash\n";

    std::cout
        << "    tr file1 file2        "
        << "Đưa nhiều file vào trash\n";

    std::cout
        << "    tr -r mydir           "
        << "Đưa mydir vào trash\n";

    std::cout
        << "    tr -f test.txt        "
        << "Xóa vĩnh viễn test.txt\n";

    std::cout
        << "    tr -rf mydir          "
        << "Xóa vĩnh viễn mydir\n";

    std::cout
        << "    tr -rf mydir/*        "
        << "Xóa nội dung mydir, giữ mydir\n";

    std::cout
        << "    tr -rf *              "
        << "Xóa toàn bộ nội dung cwd\n";

    std::cout
        << "    tr -l                 "
        << "Xem danh sách trash\n";

    std::cout
        << "    tr -re <ID>           "
        << "Khôi phục item\n";

    std::cout
        << "    tr -ref <ID>          "
        << "Khôi phục và ghi đè\n";

    std::cout << "\n";

    std::cout
        << "  " << color::gray
        << "https://github.com/dotlinux26/tr-cli"
        << color::reset
        << "\n\n";
}

void print_version() {
    print_banner();

    std::cout
        << "  " << color::cyan
        << "tr-cli"
        << color::reset
        << " phiên bản (version) "
        << color::green
        << "1.0.0"
        << color::reset
        << "\n";

    std::cout
        << "  Ngôn ngữ (Language): C++17\n"
        << "  Nền tảng (Platform): Linux (POSIX)\n"
        << "  Giấy phép (License): MIT\n";

    std::cout << "\n";
}

// ============================================================
// Error helpers
// ============================================================

void print_error(const std::string& message) {
    std::cerr
        << "  "
        << color::red
        << "Error:"
        << color::reset
        << " "
        << message
        << "\n";
}

void print_success(const std::string& message) {
    std::cout
        << "  "
        << color::green
        << message
        << color::reset
        << "\n";
}

// ============================================================
// Path helpers
// ============================================================

bool path_is_directory(const fs::path& path) {
    std::error_code ec;

    fs::file_status status =
        fs::symlink_status(path, ec);

    if (ec) {
        return false;
    }

    return fs::is_directory(status);
}

bool path_exists(const fs::path& path) {
    std::error_code ec;

    fs::file_status status =
        fs::symlink_status(path, ec);

    if (ec) {
        return false;
    }

    return fs::exists(status) ||
           fs::is_symlink(status);
}

// ============================================================
// Permanent delete helpers
// ============================================================

bool permanently_delete_many(
    const std::vector<std::string>& paths) {

    bool all_success = true;

    for (const auto& raw_path : paths) {

        fs::path path(raw_path);

        if (!path_exists(path)) {
            print_error(
                "Path not found: " + raw_path
            );

            all_success = false;
            continue;
        }

        if (!trashcli::permanent_delete(raw_path)) {
            all_success = false;
        }
    }

    return all_success;
}

bool permanently_delete_directory_contents(
    const fs::path& directory) {

    if (!path_exists(directory)) {
        print_error(
            "Directory not found: "
            + directory.string()
        );

        return false;
    }

    if (!path_is_directory(directory)) {
        print_error(
            "Not a directory: "
            + directory.string()
        );

        return false;
    }

    bool all_success = true;

    std::error_code ec;

    fs::directory_iterator it(
        directory,
        fs::directory_options::skip_permission_denied,
        ec
    );

    if (ec) {
        print_error(
            "Cannot open directory "
            + directory.string()
            + ": "
            + ec.message()
        );

        return false;
    }

    for (const auto& entry : it) {

        if (!trashcli::permanent_delete(
                entry.path().string())) {

            all_success = false;
        }
    }

    return all_success;
}

// ============================================================
// -rf argument handling
// ============================================================

bool handle_rf_arguments(
    int argc,
    char* argv[]) {

    if (argc < 3) {
        print_error(
            "-rf requires a path or wildcard pattern"
        );

        return false;
    }

    /*
     * Important:
     *
     * Shell expands:
     *
     *     tr -rf mydir/*
     *
     * into:
     *
     *     tr -rf mydir/file1 mydir/file2 ...
     *
     * Therefore we must support MORE THAN ONE argv after -rf.
     */

    std::string first_arg = argv[2];

    /*
     * --------------------------------------------------------
     * Case 1:
     *
     * tr -rf *
     *
     * If shell expands *, argv[2..] become all entries in cwd.
     *
     * We simply permanently delete every supplied path.
     * The cwd itself is never passed as a deletion target.
     * --------------------------------------------------------
     */
    if (first_arg != "*" &&
        argc >= 3) {

        /*
         * Normal multi-argument case:
         *
         * tr -rf file1 file2 file3
         *
         * OR shell-expanded:
         *
         * tr -rf mydir/a mydir/b mydir/c
         */
        if (argc > 3) {

            std::vector<std::string> paths;

            for (int i = 2; i < argc; ++i) {

                std::string arg = argv[i];

                /*
                 * If wildcard was not expanded by shell,
                 * we cannot reliably enumerate it ourselves
                 * without implementing glob matching.
                 *
                 * Normally Linux shells expand it.
                 */
                if (arg == "*") {
                    continue;
                }

                paths.push_back(arg);
            }

            if (paths.empty()) {
                print_error(
                    "No paths matched the wildcard"
                );

                return false;
            }

            return permanently_delete_many(paths);
        }
    }

    /*
     * --------------------------------------------------------
     * Case 2:
     *
     * tr -rf *
     *
     * Only happens when "*" reached the application itself,
     * e.g. shell globbing was disabled/quoted.
     * --------------------------------------------------------
     */
    if (first_arg == "*") {

        return permanently_delete_directory_contents(
            fs::current_path()
        );
    }

    /*
     * --------------------------------------------------------
     * Case 3:
     *
     * Shell normally expands:
     *
     * tr -rf mydir/*
     *
     * into multiple arguments.
     *
     * However, if quoted:
     *
     * tr -rf 'mydir/*'
     *
     * we can explicitly detect the trailing /*
     * and preserve the parent directory.
     * --------------------------------------------------------
     */
    constexpr const char* suffix = "/*";

    if (first_arg.size() >= 2 &&
        first_arg.compare(
            first_arg.size() - 2,
            2,
            suffix
        ) == 0) {

        std::string directory =
            first_arg.substr(
                0,
                first_arg.size() - 2
            );

        if (directory.empty()) {
            directory = ".";
        }

        return permanently_delete_directory_contents(
            fs::path(directory)
        );
    }

    /*
     * --------------------------------------------------------
     * Case 4:
     *
     * tr -rf directory
     * tr -rf file
     *
     * Permanently delete the supplied path itself.
     * --------------------------------------------------------
     */
    return trashcli::permanent_delete(first_arg);
}

// ============================================================
// Restore argument
// ============================================================

int handle_restore(
    const std::string& id,
    bool force) {

    if (id.empty()) {
        print_error("Trash ID cannot be empty");
        return 1;
    }

    return trashcli::restore_file(
        id,
        force
    ) ? 0 : 1;
}

// ============================================================
// Trash one or more normal paths
// ============================================================

int handle_normal_paths(
    int argc,
    char* argv[]) {

    if (argc < 2) {
        print_usage();
        return 0;
    }

    bool all_success = true;

    /*
     * Shell glob expansion means:
     *
     *     tr *.txt
     *
     * can arrive as:
     *
     *     tr a.txt b.txt c.txt
     *
     * Support all of them.
     */
    for (int i = 1; i < argc; ++i) {

        std::string path_string =
            argv[i];

        if (path_string.empty()) {
            continue;
        }

        /*
         * Options should have been handled by main().
         */
        if (!path_string.empty() &&
            path_string[0] == '-') {

            print_error(
                "Unknown option: "
                + path_string
            );

            all_success = false;
            continue;
        }

        fs::path path(path_string);

        if (!path_exists(path)) {

            print_error(
                "File not found: "
                + path_string
            );

            all_success = false;
            continue;
        }

        /*
         * Normal `tr <file>` is intentionally file-only.
         * Directories require -r.
         */
        if (path_is_directory(path)) {

            print_error(
                "Path is a directory: "
                + path_string
                + ". Use -r."
            );

            all_success = false;
            continue;
        }

        if (!trashcli::trash_file(
                path_string)) {

            all_success = false;
        }
    }

    return all_success ? 0 : 1;
}

// ============================================================
// List
// ============================================================

int handle_list() {

    auto entries =
        trashcli::list_trash();

    if (entries.empty()) {

        std::cout
            << "  "
            << color::yellow
            << "Thùng rác trống (Trash is empty)."
            << color::reset
            << "\n\n";

        return 0;
    }

    std::cout
        << "  "
        << color::yellow
        << "Danh sách thùng rác (Trash list):"
        << color::reset
        << "\n";

    /*
     * We now have trash_path like:
     *
     * ~/.local/share/trash/<ID>
     *
     * but keep this parser backward-compatible with old
     * paths that may contain /data.
     */
    std::printf(
        "  \033[1;36m%-30s %-22s %-30s %-14s %18s\033[0m\n",
        "ID",
        "Deleted Date",
        "Original Path",
        "Type",
        "Size"
    );

    for (const auto& e : entries) {

        std::string id =
            fs::path(e.trash_path).filename().string();

        /*
         * Backward compatibility for old metadata:
         *
         * path/.../<ID>/data
         */
        if (id == "data") {
            fs::path data_path(e.trash_path);

            if (data_path.has_parent_path()) {
                id =
                    data_path.parent_path()
                        .filename()
                        .string();
            }
        }

        /*
         * Human-readable size.
         */
        std::string size_str;

        constexpr std::size_t KB = 1024ULL;
        constexpr std::size_t MB = KB * 1024ULL;
        constexpr std::size_t GB = MB * 1024ULL;
        constexpr std::size_t TB = GB * 1024ULL;

        if (e.size >= TB) {

            std::ostringstream oss;
            oss << std::fixed
                << std::setprecision(1)
                << static_cast<double>(e.size) / TB
                << " TB";

            size_str = oss.str();

        } else if (e.size >= GB) {

            std::ostringstream oss;
            oss << std::fixed
                << std::setprecision(1)
                << static_cast<double>(e.size) / GB
                << " GB";

            size_str = oss.str();

        } else if (e.size >= MB) {

            std::ostringstream oss;
            oss << std::fixed
                << std::setprecision(1)
                << static_cast<double>(e.size) / MB
                << " MB";

            size_str = oss.str();

        } else if (e.size >= KB) {

            std::ostringstream oss;
            oss << std::fixed
                << std::setprecision(1)
                << static_cast<double>(e.size) / KB
                << " KB";

            size_str = oss.str();

        } else {

            size_str =
                std::to_string(e.size) + " B";
        }

        /*
         * Truncate path from the LEFT to preserve the filename.
         */
        std::string path_display =
            e.original_path;

        constexpr std::size_t PATH_WIDTH = 30;

        if (path_display.length() > PATH_WIDTH) {

            path_display =
                "..."
                + path_display.substr(
                    path_display.length() - 27
                );
        }

        std::printf(
            "  %-30s %-22s %-30s %-14s %18s\n",
            id.c_str(),
            e.deleted_at.c_str(),
            path_display.c_str(),
            e.type.c_str(),
            size_str.c_str()
        );
    }

    std::cout << "\n";

    return 0;
}

// ============================================================
// Empty trash
// ============================================================

int handle_empty() {

    std::cout
        << "  "
        << color::yellow
        << "Bạn có chắc chắn muốn xóa toàn bộ thùng rác? "
           "(Are you sure you want to empty trash?) [y/N] "
        << color::reset;

    std::string answer;

    std::getline(
        std::cin >> std::ws,
        answer
    );

    if (answer.empty() ||
        (answer[0] != 'y' &&
         answer[0] != 'Y')) {

        std::cout
            << "  "
            << color::yellow
            << "Đã hủy (Cancelled)."
            << color::reset
            << "\n\n";

        return 0;
    }

    if (!trashcli::empty_trash(true)) {

        print_error(
            "Failed to completely empty trash."
        );

        return 1;
    }

    print_success(
        "Đã xóa toàn bộ thùng rác "
        "(Trash emptied)."
    );

    std::cout << "\n";

    return 0;
}

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[]) {

    /*
     * No arguments.
     */
    if (argc < 2) {
        print_usage();
        return 0;
    }

    std::string cmd =
        argv[1];

    // --------------------------------------------------------
    // Help / version
    // --------------------------------------------------------

    if (cmd == "-h" ||
        cmd == "--help") {

        print_usage();
        return 0;
    }

    if (cmd == "-v" ||
        cmd == "--version") {

        print_version();
        return 0;
    }

    // --------------------------------------------------------
    // List
    // --------------------------------------------------------

    if (cmd == "-l") {

        if (argc != 2) {
            print_error(
                "-l does not accept additional arguments"
            );

            return 1;
        }

        return handle_list();
    }

    // --------------------------------------------------------
    // Empty trash
    // --------------------------------------------------------

    if (cmd == "-e") {

        if (argc != 2) {
            print_error(
                "-e does not accept additional arguments"
            );

            return 1;
        }

        return handle_empty();
    }

    // --------------------------------------------------------
    // Recursive trash
    // --------------------------------------------------------

    if (cmd == "-r") {

        if (argc < 3) {

            print_error(
                "-r requires a directory"
            );

            return 1;
        }

        bool all_success = true;

        /*
         * Support shell-expanded arguments too.
         *
         * Example:
         *
         * tr -r dir/*
         *
         * If dir contains:
         *
         * a
         * b
         *
         * shell gives:
         *
         * argv = ["tr", "-r", "dir/a", "dir/b"]
         *
         * We process each target individually.
         */
        for (int i = 2; i < argc; ++i) {

            fs::path path(argv[i]);

            if (!path_exists(path)) {

                print_error(
                    "Path not found: "
                    + path.string()
                );

                all_success = false;
                continue;
            }

            if (!path_is_directory(path)) {

                print_error(
                    "-r requires a directory: "
                    + path.string()
                );

                all_success = false;
                continue;
            }

            if (!trashcli::trash_recursive(
                    path.string())) {

                all_success = false;
            }
        }

        return all_success ? 0 : 1;
    }

    // --------------------------------------------------------
    // Permanent delete
    // --------------------------------------------------------

    if (cmd == "-f") {

        if (argc < 3) {

            print_error(
                "-f requires at least one path"
            );

            return 1;
        }

        std::vector<std::string> paths;

        for (int i = 2; i < argc; ++i) {
            paths.emplace_back(argv[i]);
        }

        return permanently_delete_many(paths)
            ? 0
            : 1;
    }

    // --------------------------------------------------------
    // Recursive permanent delete
    // --------------------------------------------------------

    if (cmd == "-rf") {
        return handle_rf_arguments(
            argc,
            argv
        ) ? 0 : 1;
    }

    // --------------------------------------------------------
    // Restore
    // --------------------------------------------------------

    if (cmd == "-re") {

        if (argc != 3) {

            print_error(
                "Usage: tr -re <ID>"
            );

            return 1;
        }

        return handle_restore(
            argv[2],
            false
        );
    }

    // --------------------------------------------------------
    // Restore + overwrite
    // --------------------------------------------------------

    if (cmd == "-ref") {

        if (argc != 3) {

            print_error(
                "Usage: tr -ref <ID>"
            );

            return 1;
        }

        return handle_restore(
            argv[2],
            true
        );
    }

    // --------------------------------------------------------
    // Info
    // --------------------------------------------------------

    if (cmd == "-i") {

        if (argc != 3) {

            print_error(
                "Usage: tr -i <path>"
            );

            return 1;
        }

        return trashcli::show_info(
            argv[2],
            false
        ) ? 0 : 1;
    }

    // --------------------------------------------------------
    // Detailed info
    // --------------------------------------------------------

    if (cmd == "-info") {

        if (argc != 3) {

            print_error(
                "Usage: tr -info <path>"
            );

            return 1;
        }

        return trashcli::show_info(
            argv[2],
            true
        ) ? 0 : 1;
    }

    // --------------------------------------------------------
    // Unknown option
    // --------------------------------------------------------

    if (!cmd.empty() &&
        cmd[0] == '-') {

        print_error(
            "Unknown option: "
            + cmd
        );

        std::cout << "\n";

        print_usage();

        return 1;
    }

    // --------------------------------------------------------
    // Normal trash operation
    //
    // Supports:
    //
    //     tr file.txt
    //     tr a.txt b.txt c.txt
    //     tr *.txt
    //
    // Directory requires -r.
    // --------------------------------------------------------

    return handle_normal_paths(
        argc,
        argv
    );
}

