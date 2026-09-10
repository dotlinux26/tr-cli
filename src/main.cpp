#include "trashcli.h"
#include <iostream>
#include <string>

void print_banner() {
    std::cout << "\n";
    std::cout << "  \033[1;32m";
    std::cout << "  ████████╗███████╗██████╗ ███╗   ███╗\n";
    std::cout << "  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║\n";
    std::cout << "     ██║   █████╗  ██████╔╝██╔████╔██║\n";
    std::cout << "     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║\n";
    std::cout << "     ██║   ███████╗██║  ██║██║ ╚═╝ ██║\n";
    std::cout << "     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝\n";
    std::cout << "  \033[0m";
    std::cout << "\n";
    std::cout << "  \033[1;36mTrash CLI\033[0m - Cong cu quan ly thung rac cho Linux\n";
    std::cout << "  \033[90mPhien ban 1.0.0 | (c) 2026 Nguyễn Đức Cảnh\033[0m\n";
    std::cout << "\n";
}

void print_usage() {
    print_banner();
    std::cout << "  \033[1;33mSu dung:\033[0m\n";
    std::cout << "    tr <file>             Xoa file vao thung rac\n";
    std::cout << "    tr -r <thu muc>       Xoa de quy thu muc\n";
    std::cout << "    tr -rf <file>         Xoa vinh vien file\n";
    std::cout << "    tr -rf *              Xoa noi dung, giu thu muc\n";
    std::cout << "    tr -re <duong dan>    Khoi phuc file\n";
    std::cout << "    tr -ref <duong dan>   Khoi phuc ghi de\n";
    std::cout << "    tr -l                 Liet ke file trong trash\n";
    std::cout << "    tr -i <file>          Xem thong tin file\n";
    std::cout << "    tr -info <file>       Xem thong tin chi tiet\n";
    std::cout << "    tr -e                 Xoa toan bo trash\n";
    std::cout << "    tr -h, --help         Hien thi huong dan\n";
    std::cout << "    tr -v, --version      Hien thi phien ban\n";
    std::cout << "\n";
    std::cout << "  \033[1;33mVi du:\033[0m\n";
    std::cout << "    tr test.txt           Xoa file test.txt vao trash\n";
    std::cout << "    tr -r mydir           Xoa thu muc mydir de quy\n";
    std::cout << "    tr -l                 Xem danh sach trash\n";
    std::cout << "    tr -re /tmp/trash/... Khoi phuc file\n";
    std::cout << "\n";
    std::cout << "  \033[90mhttps://github.com/dotlinux26/tr-cli\033[0m\n";
    std::cout << "\n";
}

void print_version() {
    print_banner();
    std::cout << "  \033[1;36mtr-cli\033[0m phien ban \033[1;32m1.0.0\033[0m\n";
    std::cout << "  Ngon ngu: C++17\n";
    std::cout << "  Nen tang: Linux (POSIX)\n";
    std::cout << "  Giay phep: MIT\n";
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
        print_banner();
        auto entries = trashcli::list_trash();
        if (entries.empty()) {
            std::cout << "  \033[33mThung rac trong.\033[0m\n\n";
            return 0;
        }
        std::cout << "  \033[1;33mDanh sach thung rac:\033[0m\n";
        std::cout << "  \033[90m─────────────────────────────────────────────────────\033[0m\n";
        for (const auto& e : entries) {
            std::cout << "  \033[1;32m" << e.name << "\033[0m\n";
            std::cout << "    Goc:    " << e.original_path << "\n";
            std::cout << "    Xoa:    " << e.deleted_at << "\n\n";
        }
        return 0;
    }

    if (cmd == "-e") {
        print_banner();
        std::cout << "  \033[33mBan co chac chan muon xoa toan bo thung rac? [y/N] \033[0m";
        char c;
        std::cin >> c;
        if (c == 'y' || c == 'Y') {
            trashcli::empty_trash();
            std::cout << "  \033[32mDa xoa toan bo thung rac.\033[0m\n\n";
        } else {
            std::cout << "  \033[33mDa huy.\033[0m\n\n";
        }
        return 0;
    }

    if (cmd == "-r" && argc >= 3) {
        print_banner();
        return trashcli::trash_recursive(argv[2]) ? 0 : 1;
    }

    if (cmd == "-rf" && argc >= 3) {
        print_banner();
        if (std::string(argv[2]) == "*") {
            return trashcli::empty_trash(false) ? 0 : 1;
        }
        return trashcli::permanent_delete(argv[2]) ? 0 : 1;
    }

    if (cmd == "-re" && argc >= 3) {
        print_banner();
        return trashcli::restore_file(argv[2], false) ? 0 : 1;
    }

    if (cmd == "-ref" && argc >= 3) {
        print_banner();
        return trashcli::restore_file(argv[2], true) ? 0 : 1;
    }

    if (cmd == "-i" && argc >= 3) {
        print_banner();
        return trashcli::show_info(argv[2], false) ? 0 : 1;
    }

    if (cmd == "-info" && argc >= 3) {
        print_banner();
        return trashcli::show_info(argv[2], true) ? 0 : 1;
    }

    if (cmd[0] != '-') {
        print_banner();
        return trashcli::trash_file(cmd) ? 0 : 1;
    }

    print_usage();
    return 1;
}
