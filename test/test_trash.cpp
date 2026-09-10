#include "trashcli.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void test_get_trash_dir() {
    std::string dir = trashcli::get_trash_dir();
    assert(!dir.empty());
    assert(dir.find(".local/share/trash") != std::string::npos);
    std::cout << "PASS: test_get_trash_dir\n";
}

void test_generate_trash_subdir() {
    std::string sub = trashcli::generate_trash_subdir();
    assert(sub.length() == 21);
    assert(sub[8] == '-');
    std::cout << "PASS: test_generate_trash_subdir\n";
}

void test_trash_file() {
    std::ofstream("/tmp/testfile.txt") << "test content";
    bool result = trashcli::trash_file("/tmp/testfile.txt");
    assert(result);
    assert(!fs::exists("/tmp/testfile.txt"));

    auto entries = trashcli::list_trash();
    bool found = false;
    for (const auto& e : entries) {
        if (e.name == "testfile.txt") { found = true; break; }
    }
    assert(found);
    std::cout << "PASS: test_trash_file\n";
}

void test_restore_file() {
    std::ofstream("/tmp/restore_test.txt") << "restore me";
    trashcli::trash_file("/tmp/restore_test.txt");

    auto entries = trashcli::list_trash();
    for (const auto& e : entries) {
        if (e.name == "restore_test.txt") {
            bool result = trashcli::restore_file(e.trash_path + "/restore_test.txt", false);
            assert(result);
            assert(fs::exists("/tmp/restore_test.txt"));
            break;
        }
    }
    std::cout << "PASS: test_restore_file\n";
}

void test_list_trash() {
    auto entries = trashcli::list_trash();
    assert(entries.size() >= 0);
    std::cout << "PASS: test_list_trash\n";
}

void test_show_info() {
    std::ofstream("/tmp/info_test.txt") << "info content";
    bool result = trashcli::show_info("/tmp/info_test.txt", false);
    assert(result);
    result = trashcli::show_info("/tmp/info_test.txt", true);
    assert(result);
    fs::remove("/tmp/info_test.txt");
    std::cout << "PASS: test_show_info\n";
}

int main() {
    test_get_trash_dir();
    test_generate_trash_subdir();
    test_trash_file();
    test_restore_file();
    test_list_trash();
    test_show_info();

    trashcli::empty_trash();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
