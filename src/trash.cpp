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

namespace fs = std::filesystem;

namespace trashcli {

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

bool create_metadata(const std::string& trash_path, const std::string& original_path) {
    std::string meta_path = trash_path + "/metadata.json";

    struct stat st;
    stat(original_path.c_str(), &st);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    localtime_r(&time, &tm_buf);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

    fs::path orig(original_path);

    std::ofstream ofs(meta_path);
    if (!ofs.is_open()) return false;

    ofs << "{\n";
    ofs << "  \"name\": \"" << orig.filename().string() << "\",\n";
    ofs << "  \"original_path\": \"" << original_path << "\",\n";
    ofs << "  \"deleted_at\": \"" << time_str << "\",\n";
    ofs << "  \"size\": " << st.st_size << "\n";
    ofs << "}\n";

    return ofs.good();
}

bool trash_file(const std::string& filepath) {
    fs::path fp(filepath);
    if (!fs::exists(fp)) {
        std::cerr << "Error: File not found: " << filepath << "\n";
        return false;
    }

    std::string trash_base = get_trash_dir();
    if (trash_base.empty()) return false;

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
    return create_metadata(trash_dir, fs::absolute(fp).string());
}

bool trash_recursive(const std::string& dirpath) {
    fs::path dp(dirpath);
    if (!fs::exists(dp) || !fs::is_directory(dp)) {
        std::cerr << "Error: Directory not found: " << dirpath << "\n";
        return false;
    }

    std::string trash_base = get_trash_dir();
    if (trash_base.empty()) return false;

    std::string subdir = trash_base + "/" + generate_trash_subdir() + "/data";
    fs::create_directories(subdir);

    fs::path dest = fs::path(subdir) / dp.filename();
    fs::rename(dp, dest);

    std::string trash_dir = fs::path(subdir).parent_path().string();
    return create_metadata(trash_dir, fs::absolute(dp).string());
}

bool restore_file(const std::string& trash_path, bool force) {
    fs::path tp(trash_path);
    if (!fs::exists(tp)) {
        std::cerr << "Error: File not found in trash: " << trash_path << "\n";
        return false;
    }

    std::string meta_path = fs::path(trash_path).parent_path().parent_path().string() + "/metadata.json";
    std::ifstream ifs(meta_path);
    if (!ifs.is_open()) {
        std::cerr << "Error: Metadata not found\n";
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());

    auto pos = content.find("\"original_path\": \"");
    if (pos == std::string::npos) {
        std::cerr << "Error: Invalid metadata\n";
        return false;
    }
    pos += 18;
    auto end = content.find("\"", pos);
    std::string original = content.substr(pos, end - pos);

    if (fs::exists(original) && !force) {
        std::cerr << "Error: File already exists at: " << original << "\n";
        std::cerr << "Use -ref to overwrite.\n";
        return false;
    }

    fs::create_directories(fs::path(original).parent_path());
    fs::rename(tp, original);

    fs::remove_all(tp.parent_path());
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
    std::string trash_base = get_trash_dir();
    if (!fs::exists(trash_base)) return entries;

    for (auto& entry : fs::directory_iterator(trash_base)) {
        if (!entry.is_directory()) continue;

        std::string data_dir = entry.path().string() + "/data";
        std::string meta_path = entry.path().string() + "/metadata.json";

        if (!fs::exists(data_dir)) continue;

        TrashEntry te;
        te.trash_path = data_dir;

        std::ifstream ifs(meta_path);
        if (ifs.is_open()) {
            std::string content((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());

            auto p1 = content.find("\"name\": \"");
            if (p1 != std::string::npos) {
                p1 += 9;
                auto e1 = content.find("\"", p1);
                te.name = content.substr(p1, e1 - p1);
            }

            auto p2 = content.find("\"original_path\": \"");
            if (p2 != std::string::npos) {
                p2 += 17;
                auto e2 = content.find("\"", p2);
                te.original_path = content.substr(p2, e2 - p2);
            }

            auto p3 = content.find("\"deleted_at\": \"");
            if (p3 != std::string::npos) {
                p3 += 15;
                auto e3 = content.find("\"", p3);
                te.deleted_at = content.substr(p3, e3 - p3);
            }
        }

        if (fs::exists(data_dir)) {
            for (auto& f : fs::recursive_directory_iterator(data_dir)) {
                te.size += f.file_size();
            }
        }

        entries.push_back(te);
    }

    std::sort(entries.begin(), entries.end(), [](const TrashEntry& a, const TrashEntry& b) {
        return a.deleted_at > b.deleted_at;
    });

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
// Issue #1: Bo sung ham tao metadata.json
