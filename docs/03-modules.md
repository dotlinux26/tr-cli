# 03. Thiết kế module

## 3.1. Tổng quan kiến trúc module

```
┌─────────────────────────────────────────┐
│              trashcli                   │
├─────────────┬───────────────────────────┤
│  Module     │  Chức năng                │
├─────────────┼───────────────────────────┤
│ CLI Parser  │  Phân tích tham số dòng lệnh│
│ Trash Core  │  Xử lý nghiệp vụ chính     │
│ Database    │  SQLite3 metadata storage │
│ File System │  Thao tác file POSIX      │
└─────────────┴───────────────────────────┘
```

## 3.2. Module CLI Parser (src/main.cpp)

**Trách nhiệm:** Phân tích tham số dòng lệnh và chuyển hướng đến hàm xử lý tương ứng.

**Mapping table:**

| Flag | Hàm gọi | Mô tả |
|------|---------|-------|
| (không) | `trash_file()` | Xóa file vào trash |
| `-r` | `trash_recursive()` | Xóa đệ quy thư mục |
| `-rf` | `permanent_delete()` | Xóa vĩnh viễn |
| `-rf *` | `empty_keep_dir()` (cwd) | Xóa nội dung, giữ thư mục hiện tại |
| `-rf <dir>/*` | `empty_keep_dir()` (dir) | Xóa nội dung thư mục chỉ định |
| `-re` | `restore_file(false)` | Khôi phục theo ID |
| `-ref` | `restore_file(true)` | Khôi phục ghi đè theo ID |
| `-l` | `list_trash()` | Liệt kê file trong trash |
| `-i` | `show_info(false)` | Xem thông tin cơ bản |
| `-info` | `show_info(true)` | Xem thông tin chi tiết |
| `-e` | `empty_trash()` | Xóa toàn bộ trash |
| `-h/--help` | `print_usage()` | Hiển thị hướng dẫn |
| `-v/--version` | `print_version()` | Hiển thị phiên bản |

## 3.3. Module Trash Core (src/trash.cpp)

**Trách nhiệm:** Các hàm chính xử lý xóa, khôi phục, liệt kê, xóa vĩnh viễn.

| Hàm | Input | Output | Mô tả |
|-----|-------|--------|-------|
| `trash_file(filepath)` | string | bool | Di chuyển file vào trash, insert SQLite |
| `trash_recursive(dirpath)` | string | bool | Di chuyển thư mục đệ quy vào trash |
| `restore_file(trash_id, force)` | string, bool | bool | Khôi phục theo ID, query file_type |
| `permanent_delete(filepath)` | string | bool | Xóa vĩnh viễn (unlink/remove_all) |
| `empty_trash(keep_root)` | bool | bool | Xóa toàn bộ trash + SQLite records |
| `list_trash()` | - | vector<TrashEntry> | SELECT ORDER BY deleted_at DESC |
| `show_info(filepath, detailed)` | string, bool | bool | stat() + hiển thị thông tin |

## 3.4. Module Database (src/trash.cpp)

**File DB:** `~/.local/share/trash/trash.db`

**Hàm chính:**

| Hàm | Mô tả |
|-----|-------|
| `init_database()` | Mở/kết nối DB, CREATE TABLE IF NOT EXISTS |
| `insert_trash_entry()` | INSERT INTO trash_entries (prepared stmt) |
| `delete_trash_entry(trash_id)` | DELETE WHERE trash_path LIKE ? |
| `list_trash()` | SELECT ... ORDER BY deleted_at DESC |

**Prepared statements** dùng cho mọi truy vấn (tránh SQL injection, tối ưu).

## 3.5. Module File System

**Sử dụng std::filesystem (C++17):**

| API | Mục đích |
|-----|----------|
| `exists()` | Kiểm tra file/thư mục tồn tại |
| `rename()` | Di chuyển file/thư mục (atomic) |
| `remove()` | Xóa file đơn |
| `remove_all()` | Xóa đệ quy thư mục |
| `create_directories()` | Tạo thư mục đệ quy |
| `directory_iterator()` | Duyệt thư mục (non-recursive) |
| `recursive_directory_iterator()` | Duyệt đệ quy |
| `absolute()` | Lấy đường dẫn tuyệt đối |
| `current_path()` | Thư mục làm việc hiện tại |
| `path::filename()`, `stem()`, `extension()` | Phân tích tên file |

## 3.6. Struct TrashEntry (include/trashcli.h)

```cpp
struct TrashEntry {
    std::string name;          // Tên file/thư mục
    std::string original_path; // Đường dẫn gốc
    std::string trash_path;    // Đường dẫn session trong trash
    std::string deleted_at;    // Thời điểm xóa (YYYY-MM-DD HH:MM:SS)
    std::string type;          // "file" hoặc "directory"
    std::size_t size;          // Kích thước (bytes)
};
```

## Kết luận chương 3
Chương 3 đã trình bày thiết kế 4 module chính của Trash CLI: CLI Parser, Trash Core, Database (SQLite3), File System. Mỗi module có trách nhiệm rõ ràng, tách biệt điểm quan tâm (separation of concerns). Module Database sử dụng SQLite3 embedded thay vì JSON file, khắc phục Bug #6. Module Trash Core xử lý restore file/directory phân biệt nhờ query `file_type` (khắc phục Bug #8).