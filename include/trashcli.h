#ifndef TRASHCLI_H
#define TRASHCLI_H

#include <string>
#include <vector>

namespace trashcli {

struct TrashEntry {
    std::string name;
    std::string original_path;
    std::string trash_path;
    std::string deleted_at;
    std::string type;  // "file" hoặc "directory"
    std::size_t size;
};

std::string get_trash_dir();
std::string generate_trash_subdir();
bool create_metadata(const std::string& trash_path, const std::string& original_path);
bool trash_file(const std::string& filepath);
bool trash_recursive(const std::string& dirpath);
bool restore_file(const std::string& trash_path, bool force = false);
bool permanent_delete(const std::string& filepath);
bool empty_trash(bool keep_root = true);
std::vector<TrashEntry> list_trash();
bool show_info(const std::string& filepath, bool detailed = false);

}

#endif
