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
│ Database    │  SQLite3 metadata         │
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
    ├── -rf *       → xóa nội dung thư mục hiện tại
    ├── -rf <dir>/* → xóa nội dung thư mục chỉ định
    ├── -re <ID>    → restore_file(ID, false)
    ├── -ref <ID>   → restore_file(ID, true)
    ├── -i <ID>     → show_info(ID, false)
    ├── -info <ID>  → show_info(ID, true)
    └── <file>      → trash_file(file)
```

**Bảng mapping:**

| Flag | Hàm gọi | Mô tả |
|------|---------|-------|
| (không) | trash_file() | Xóa file vào trash |
| -r | trash_recursive() | Xóa đệ quy |
| -rf | permanent_delete() | Xóa vĩnh viễn |
| -rf * | xóa nội dung thư mục hiện tại | Giữ thư mục |
| -rf <dir>/* | xóa nội dung thư mục | Giữ thư mục |
| -re | restore_file() | Khôi phục theo ID |
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
| restore_file(trash_id, force) | string, bool | bool | Khôi phục file theo ID |
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
    ├── Lấy size + absolute path (trước khi move)
    │
    ├── Tạo thư mục trash
    │   └── ~/.local/share/trash/YYYYMMDD-HHmmss-5hex/data/
    │
    ├── Di chuyển file
    │   ├── Nếu file đã tồn tại → thêm _1, _2, ...
    │   └── fs::rename(file, dest)
    │
    └── Lưu metadata vào SQLite3 (trash_entries)
        └── Ghi tên, đường dẫn gốc, ngày xóa, kích thước, loại
```

**Luồng xử lý của restore_file(ID):**

```
restore_file(trash_id, force)
    │
    ├── Tìm session trong ~/.local/share/trash/{ID}/
    │
    ├── Query SQLite3 lấy original_path
    │   └── SELECT original_path FROM trash_entries WHERE trash_path LIKE ?
    │
    ├── Kiểm tra xung đột tại original_path
    │   └── Nếu tồn tại và !force → báo lỗi, yêu cầu -ref
    │
    ├── Di chuyển file từ data/ về original_path
    │   └── fs::rename(data/*, original_path)
    │
    └── Xóa record khỏi SQLite3 + xóa thư mục session
```

## 3.4. Module Database (SQLite3)

**Trách nhiệm:** Quản lý metadata thông qua SQLite3 embedded database.

**File:** `src/trash.cpp` (hàm `init_database`, `insert_trash_entry`, `delete_trash_entry`, `list_trash`)

**Cơ sở dữ liệu:** `~/.local/share/trash/trash.db`

**Bảng trash_entries:**
```sql
CREATE TABLE trash_entries (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    original_path TEXT NOT NULL,
    trash_path TEXT NOT NULL,
    deleted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    size INTEGER DEFAULT 0,
    file_type TEXT DEFAULT 'file'
);
```

**Bảng trash_sessions:**
```sql
CREATE TABLE trash_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_dir TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

**Indexes:**
```sql
CREATE INDEX idx_original_path ON trash_entries(original_path);
CREATE INDEX idx_deleted_at ON trash_entries(deleted_at);
CREATE INDEX idx_session_dir ON trash_sessions(session_dir);
```

**Các hàm chính:**

| Hàm | Mô tả |
|-----|-------|
| init_database() | Mở/kết nối DB, tạo bảng nếu chưa có |
| insert_trash_entry() | INSERT record mới |
| delete_trash_entry() | DELETE theo trash_id |
| list_trash() | SELECT ORDER BY deleted_at DESC |

**Prepared statements** được dùng cho mọi truy vấn (tránh SQL injection, tối ưu hiệu năng).

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
- `std::filesystem::current_path()` - Thư mục làm việc hiện tại

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
    std::string type;       // "file" hoặc "directory"
    std::size_t size;
};

std::string get_trash_dir();
std::string generate_trash_subdir();
bool trash_file(const std::string& filepath);
bool trash_recursive(const std::string& dirpath);
bool restore_file(const std::string& trash_id, bool force = false);
bool permanent_delete(const std::string& filepath);
bool empty_trash(bool keep_root = true);
std::vector<TrashEntry> list_trash();
bool show_info(const std::string& filepath, bool detailed = false);

}

#endif
```