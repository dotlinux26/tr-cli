# 03. Các module

## 3.1. Tổng quan

Hệ thống được chia thành 4 module chính:

```
┌─────────────────────────────────────────┐
│              trashcli                   │
├─────────────┬───────────────────────────┤
│  Module     │  Chức năng                │
├─────────────┼───────────────────────────┤
│ CLI Parser  │  Phân tích tham số        │
│ Trash Core  │  Xử lý chính              │
│ Metadata    │  Quản lý thông tin        │
│ File System │  Thao tác file            │
└─────────────┴───────────────────────────┘
```

## 3.2. Module CLI Parser

**File:** `src/main.cpp`

**Trách nhiệm:** Phân tích tham số dòng lệnh và chuyển hướng đến hàm tương ứng.

**Cấu trúc:**

```
main(argc, argv)
    ├── -l          → list_trash()
    ├── -e          → empty_trash()
    ├── -r <dir>    → trash_recursive(dir)
    ├── -rf <file>  → permanent_delete(file)
    ├── -rf *       → empty_trash(false)
    ├── -re <path>  → restore_file(path, false)
    ├── -ref <path> → restore_file(path, true)
    ├── -i <file>   → show_info(file, false)
    ├── -info <file>→ show_info(file, true)
    └── <file>      → trash_file(file)
```

**Bảng mapping:**

| Flag | Hàm gọi | Mô tả |
|------|---------|-------|
| (không) | trash_file() | Xóa file vào trash |
| -r | trash_recursive() | Xóa đệ quy |
| -rf | permanent_delete() | Xóa vĩnh viễn |
| -rf * | empty_trash(false) | Xóa nội dung, giữ thư mục |
| -re | restore_file() | Khôi phục |
| -ref | restore_file(force=true) | Khôi phục ghi đè |
| -l | list_trash() | Liệt kê |
| -i | show_info() | Thông tin |
| -info | show_info(detailed=true) | Thông tin chi tiết |
| -e | empty_trash() | Xóa toàn bộ trash |

## 3.3. Module Trash Core

**File:** `src/trash.cpp`

**Trách nhiệm:** Các hàm chính xử lý xóa, khôi phục, liệt kê.

**Các hàm:**

| Hàm | Input | Output | Mô tả |
|-----|-------|--------|-------|
| trash_file(filepath) | string | bool | Di chuyển file vào trash |
| trash_recursive(dirpath) | string | bool | Xóa đệ quy thư mục |
| restore_file(trash_path, force) | string, bool | bool | Khôi phục file |
| permanent_delete(filepath) | string | bool | Xóa vĩnh viễn |
| empty_trash(keep_root) | bool | bool | Xóa toàn bộ trash |
| list_trash() | - | vector | Liệt kê file |
| show_info(filepath, detailed) | string, bool | bool | Hiển thị thông tin |

**Luồng xử lý của trash_file():**

```
trash_file(filepath)
    │
    ├── Kiểm tra file tồn tại
    │   └── Nếu không → in lỗi, return false
    │
    ├── Tạo thư mục trash
    │   └── ~/.local/share/trash/YYYYMMDD-HHmmss-5hex/data/
    │
    ├── Di chuyển file
    │   ├── Nếu file đã tồn tại → thêm _1, _2, ...
    │   └── fs::rename(file, dest)
    │
    └── Tạo metadata.json
        └── Ghi tên, đường dẫn gốc, ngày xóa, kích thước
```

## 3.4. Module Metadata

**Trách nhiệm:** Tạo, đọc và quản lý file metadata.json.

**Cấu trúc metadata.json:**

```json
{
    "name": "ten_file.txt",
    "original_path": "/home/user/duongdan/ten_file.txt",
    "deleted_at": "2026-09-10 20:51:50",
    "size": 1024
}
```

**Các hàm:**

| Hàm | Mô tả |
|-----|-------|
| create_metadata() | Tạo file metadata.json |
| parse_metadata() | Đọc metadata từ file JSON |

## 3.5. Module File System

**Trách nhiệm:** Thao tác trực tiếp với hệ thống file.

**Sử dụng:**

- `std::filesystem::exists()` - Kiểm tra file tồn tại
- `std::filesystem::rename()` - Di chuyển file
- `std::filesystem::remove()` - Xóa file
- `std::filesystem::remove_all()` - Xóa đệ quy
- `std::filesystem::create_directories()` - Tạo thư mục
- `std::filesystem::directory_iterator()` - Duyệt thư mục
- `std::filesystem::recursive_directory_iterator()` - Duyệt đệ quy
- `std::filesystem::absolute()` - Lấy đường dẫn tuyệt đối

## 3.6. File header

**File:** `include/trashcli.h`

```cpp
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
    std::size_t size;
};

std::string get_trash_dir();
std::string generate_trash_subdir();
bool create_metadata(const std::string& trash_path,
                     const std::string& original_path);
bool trash_file(const std::string& filepath);
bool trash_recursive(const std::string& dirpath);
bool restore_file(const std::string& trash_path,
                  bool force = false);
bool permanent_delete(const std::string& filepath);
bool empty_trash(bool keep_root = true);
std::vector<TrashEntry> list_trash();
bool show_info(const std::string& filepath,
               bool detailed = false);

}

#endif
```
