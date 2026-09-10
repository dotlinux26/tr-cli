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
| `-r` | `trash_recursive()` | Xóa đệ quy thư mục vào trash |
| `-f` | `permanent_delete()` | Xóa vĩnh viễn file |
| `-rf` | `permanent_delete()` | Xóa vĩnh viễn thư mục |
| `-rf <dir>/*` | `empty_keep_dir()` | Xóa nội dung thư mục, giữ thư mục |
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
| `trash_recursive(dirpath)` | string | bool | Di chuyển thư mục đệ quy vào trash (normalize path) |
| `restore_file(trash_id, force)` | string, bool | bool | Khôi phục theo ID, query file_type + stored_name |
| `permanent_delete(filepath)` | string | bool | Xóa vĩnh viễn (unlink/remove_all) |
| `empty_trash(keep_root)` | bool | bool | Xóa toàn bộ trash + SQLite records |
| `list_trash()` | - | vector<TrashEntry> | SELECT ORDER BY deleted_at DESC |
| `show_info(filepath, detailed)` | string, bool | bool | stat() + hiển thị thông tin |

**Các hàm helper:**
| Hàm | Mô tả |
|-----|-------|
| `move_to_trash(src, dest, ec)` | Di chuyển file/thư mục, hỗ trợ cross-filesystem (copy+remove fallback) |
| `calculate_directory_size(path)` | Tính dung lượng thực tế của thư mục đệ quy |
| `generate_unique_trash_id()` | Tạo ID dạng YYYYMMDD-HHMMSS-6hex |
| `path_exists()`, `path_is_directory()` | Helper an toàn cho filesystem |

## 3.4. Module Database (src/trash.cpp)

**File DB:** `~/.local/share/trash/trash.db`

**Schema:**
```sql
trash_entries (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    trash_id TEXT UNIQUE NOT NULL,     -- YYYYMMDD-HHMMSS-6hex
    name TEXT NOT NULL,                -- Tên file gốc
    original_path TEXT NOT NULL,       -- Đường dẫn gốc
    trash_path TEXT NOT NULL,          -- Đường dẫn session
    stored_name TEXT NOT NULL,         -- Tên file thực trong trash (có _1, _2 nếu collision)
    deleted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    size INTEGER DEFAULT 0,
    file_type TEXT DEFAULT 'file'
);
```

**Indexes:**
```sql
CREATE INDEX idx_original_path ON trash_entries(original_path);
CREATE INDEX idx_deleted_at ON trash_entries(deleted_at);
CREATE INDEX idx_session_dir ON trash_sessions(session_dir);
CREATE INDEX idx_trash_id ON trash_entries(trash_id);  -- Quan trọng: lookup chính xác O(log n)
```

**Hàm chính:**

| Hàm | Mô tả |
|-----|-------|
| `init_database()` | Mở/kết nối DB, CREATE TABLE IF NOT EXISTS, WAL mode |
| `insert_trash_entry()` | INSERT INTO trash_entries (prepared stmt) |
| `delete_trash_entry(trash_id)` | DELETE WHERE trash_id = ? (exact match) |
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
| `lexically_normal()` | Chuẩn hóa path (loại bỏ `/./`, `//`, trailing slash) |

**Cross-filesystem support:** `move_to_trash()` tự động fallback copy+remove khi `rename()` fail do EXDEV.

## 3.6. Struct TrashEntry (include/trashcli.h)

```cpp
struct TrashEntry {
    std::string name;          // Tên file/thư mục
    std::string original_path; // Đường dẫn gốc
    std::string trash_path;    // Đường dẫn session trong trash
    std::string trash_id;      // ID duy nhất (YYYYMMDD-HHMMSS-6hex)
    std::string deleted_at;    // Thời điểm xóa (YYYY-MM-DD HH:MM:SS)
    std::string type;          // "file" hoặc "directory"
    std::size_t size;          // Kích thước (bytes)
};
```

## Kết luận chương 3
Chương 3 đã trình bày thiết kế 4 module chính của Trash CLI: CLI Parser, Trash Core, Database (SQLite3), File System. Mỗi module có trách nhiệm rõ ràng, tách biệt điểm quan tâm (separation of concerns). Module Database sử dụng SQLite3 embedded thay vì JSON file, khắc phục Bug #6. Module Trash Core xử lý restore file/directory phân biệt nhờ query `file_type` và `stored_name` (khắc phục Bug #8). Tính năng cross-filesystem, collision handling, path normalization đã được implement trong core logic.