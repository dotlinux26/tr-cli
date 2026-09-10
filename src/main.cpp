#include "trashcli.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

void print_banner() {
    std::cout << "\n";
    std::cout << "\033[1;32m";
    std::cout << "  ████████╗███████╗██████╗ ███╗   ███╗\n";
    std::cout << "  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║\n";
    std::cout << "     ██║   █████╗  ██████╔╝██╔████╔██║\n";
    std::cout << "     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║\n";
    std::cout << "     ██║   ███████╗██║  ██║██║ ╚═╝ ██║\n";
    std::cout << "     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝\n";
    std::cout << "\033[0m";
    std::cout << "\n";
    std::cout << "  \033[1;36mTrash CLI\033[0m - Công cụ quản lý thùng rác cho Linux (Trash management tool for Linux)\n";
    std::cout << "  \033[90mPhiên bản 1.0.0 (Version) | (c) 2026 Nguyễn Đức Cảnh\033[0m\n";
    std::cout << "\n";
}

void print_usage() {
    print_banner();
    std::cout << "  \033[1;33mSử dụng (Usage):\033[0m\n";
    std::cout << "    tr <file>             Xóa file vào thùng rác (Delete file to trash)\n";
    std::cout << "    tr -r <thu muc>       Xóa đệ quy thư mục (Delete directory recursively)\n";
    std::cout << "    tr -rf <file>         Xóa vĩnh viễn file (Delete file permanently)\n";
    std::cout << "    tr -rf *              Xóa nội dung, giữ thư mục (Delete contents, keep directory)\n";
    std::cout << "    tr -re <ID>           Khôi phục theo ID (Restore by ID)\n";
    std::cout << "    tr -ref <ID>          Khôi phục ghi đè theo ID (Restore and overwrite by ID)\n";
    std::cout << "    tr -l                 Liệt kê file trong trash (List files in trash)\n";
    std::cout << "    tr -i <ID>            Xem thông tin theo ID (View info by ID)\n";
    std::cout << "    tr -info <ID>         Xem thông tin chi tiết theo ID (View detailed info by ID)\n";
    std::cout << "    tr -e                 Xóa toàn bộ trash (Empty trash)\n";
    std::cout << "    tr -h, --help         Hiển thị hướng dẫn (Show help)\n";
    std::cout << "    tr -v, --version      Hiển thị phiên bản (Show version)\n";
    std::cout << "\n";
    std::cout << "  \033[1;33mVí dụ (Example):\033[0m\n";
    std::cout << "    tr test.txt           Xóa file test.txt vào trash (Delete test.txt to trash)\n";
    std::cout << "    tr -r mydir           Xóa thư mục mydir đệ quy (Delete mydir recursively)\n";
    std::cout << "    tr -l                 Xem danh sách trash (View trash list)\n";
    std::cout << "    tr -re 20260910-abc   Khôi phục file (Restore file)\n";
    std::cout << "\n";
    std::cout << "  \033[90mhttps://github.com/dotlinux26/tr-cli\033[0m\n";
    std::cout << "\n";
}

void print_version() {
    print_banner();
    std::cout << "  \033[1;36mtr-cli\033[0m phiên bản (version) \033[1;32m1.0.0\033[0m\n";
    std::cout << "  Ngôn ngữ (Language): C++17\n";
    std::cout << "  Nền tảng (Platform): Linux (POSIX)\n";
    std::cout << "  Giấy phép (License): MIT\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "-h" || cmd == "--help") {
        print_usage();
        return 0;
    }

    if (cmd == "-v" || cmd == "--version") {
        print_version();
        return 0;
    }

    if (cmd == "-l") {
        auto entries = trashcli::list_trash();
        if (entries.empty()) {
            std::cout << "  \033[33mThùng rác trống (Trash is empty).\033[0m\n\n";
            return 0;
        }
        std::cout << "  \033[1;33mDanh sách thùng rác (Trash list):\033[0m\n";
        std::cout << "  \033[90m────────────────────────────────────────────────────────────────────────────────────────────────────\033[0m\n";
        std::cout << "  \033[1;36m"
                  << std::left << std::setw(26) << "ID (Mã)"
                  << std::left << std::setw(22) << "Ngày xóa (Date)"
                  << std::left << std::setw(55) << "Đường dẫn gốc (Original Path)"
                  << std::left << std::setw(14) << "Loại (Type)"
                  << std::right << std::setw(14) << "Kích thước (Size)"
                  << "\033[0m\n";
        std::cout << "  \033[90m────────────────────────────────────────────────────────────────────────────────────────────────────\033[0m\n";
        for (const auto& e : entries) {
            std::string id = e.trash_path;
            size_t pos = id.rfind("/data");
            if (pos != std::string::npos) id = id.substr(0, pos);
            pos = id.rfind('/');
            if (pos != std::string::npos) id = id.substr(pos + 1);
            
            std::string size_str;
            if (e.size >= 1024 * 1024) size_str = std::to_string(e.size / 1024 / 1024) + " MB";
            else if (e.size >= 1024) size_str = std::to_string(e.size / 1024) + " KB";
            else size_str = std::to_string(e.size) + " B";

            // Cắt bớt đường dẫn nếu quá dài để không bị tràn cột
            std::string path_display = e.original_path;
            if (path_display.length() > 53) {
                path_display = "..." + path_display.substr(path_display.length() - 50);
            }

            std::cout << "  "
                      << std::left << std::setw(26) << id
                      << std::left << std::setw(22) << e.deleted_at
                      << std::left << std::setw(55) << path_display
                      << std::left << std::setw(14) << e.type
                      << std::right << std::setw(14) << size_str
                      << "\n";
        }
        std::cout << "\n";
        return 0;
    }

    if (cmd == "-e") {
        std::cout << "  \033[33mBạn có chắc chắn muốn xóa toàn bộ thùng rác? (Are you sure you want to empty trash?) [y/N] \033[0m";
        char c;
        std::cin >> c;
        if (c == 'y' || c == 'Y') {
            trashcli::empty_trash();
            std::cout << "  \033[32mĐã xóa toàn bộ thùng rác (Trash emptied).\033[0m\n\n";
        } else {
            std::cout << "  \033[33mĐã hủy (Cancelled).\033[0m\n\n";
        }
        return 0;
    }

    if (cmd == "-r" && argc >= 3) {
        return trashcli::trash_recursive(argv[2]) ? 0 : 1;
    }

    if (cmd == "-rf" && argc >= 3) {
        std::string arg = argv[2];
        if (arg == "*") {
            // Delete all contents of current directory but keep the directory
            fs::path cwd = fs::current_path();
            for (auto& entry : fs::directory_iterator(cwd)) {
                std::string path = entry.path().string();
                if (!trashcli::permanent_delete(path)) {
                    return 1;
                }
            }
            return 0;
        }
        // Handle folder/* pattern - delete contents but keep folder
        if (arg.size() >= 2 && arg.substr(arg.size() - 2) == "/*") {
            std::string folder = arg.substr(0, arg.size() - 1); // remove *
            fs::path fp(folder);
            if (!fs::exists(fp) || !fs::is_directory(fp)) {
                std::cerr << "Error: Directory not found: " << folder << "\n";
                return 1;
            }
            for (auto& entry : fs::directory_iterator(fp)) {
                std::string path = entry.path().string();
                if (!trashcli::permanent_delete(path)) {
                    return 1;
                }
            }
            return 0;
        }
        return trashcli::permanent_delete(argv[2]) ? 0 : 1;
    }

    if (cmd == "-re" && argc >= 3) {
        return trashcli::restore_file(argv[2], false) ? 0 : 1;
    }

    if (cmd == "-ref" && argc >= 3) {
        return trashcli::restore_file(argv[2], true) ? 0 : 1;
    }

    if (cmd == "-i" && argc >= 3) {
        return trashcli::show_info(argv[2], false) ? 0 : 1;
    }

    if (cmd == "-info" && argc >= 3) {
        return trashcli::show_info(argv[2], true) ? 0 : 1;
    }

    if (cmd[0] != '-') {
        return trashcli::trash_file(cmd) ? 0 : 1;
    }

    print_usage();
    return 1;
}