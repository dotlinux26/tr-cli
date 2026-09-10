# 02. Hệ thống SQL (SQLite3)

## 2.1. Giới thiệu

Hệ thống sử dụng **SQLite3** (embedded database) để lưu trữ metadata về các file đã xóa. SQLite3 được chọn vì:
- Tính nhẹ, không cần cấu hình server
- Hỗ trợ trực tiếp qua API C++
- Tích hợp sẵn trong hầu hết hệ thống Linux
- ACID transactions đảm bảo tính toàn vẹn dữ liệu

## 2.2. Cấu trúc bảng

### Bảng: `trash_entries`

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

Mô tả các cột:

| Cột | Kiểu dữ liệu | Mô tả |
|-----|-------------|-------|
| id | INTEGER | Mã định danh, tự tăng |
| name | TEXT | Tên file gốc |
| original_path | TEXT | Đường dẫn đầy đủ của file gốc |
| trash_path | TEXT | Đường dẫn thư mục trash (session) |
| deleted_at | TIMESTAMP | Thời điểm xóa |
| size | INTEGER | Kích thước file (byte) |
| file_type | TEXT | Loại file (file/directory) |

### Bảng: `trash_sessions`

```sql
CREATE TABLE trash_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_dir TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

Mô tả:
- `session_dir`: Tên thư mục session (format: `YYYYMMDD-HHMMSS-5hex`)
- Mỗi lần xóa tạo một session, chứa nhiều file/thư mục

## 2.3. Cấu trúc index

```sql
CREATE INDEX idx_original_path ON trash_entries(original_path);
CREATE INDEX idx_deleted_at ON trash_entries(deleted_at);
CREATE INDEX idx_session_dir ON trash_sessions(session_dir);
```

## 2.4. Các truy vấn thường dùng

### Thêm file mới vào trash

```sql
INSERT INTO trash_entries (name, original_path, trash_path, deleted_at, size, file_type)
VALUES (?, ?, ?, ?, ?, ?);
```

### Liệt kê tất cả file trong trash

```sql
SELECT id, name, original_path, trash_path, deleted_at, size, file_type
FROM trash_entries
ORDER BY deleted_at DESC;
```

### Tìm kiếm file theo tên

```sql
SELECT * FROM trash_entries
WHERE name LIKE ?
ORDER BY deleted_at DESC;
```

### Lấy thông tin file theo ID

```sql
SELECT * FROM trash_entries
WHERE id = ?;
```

### Khôi phục file (lấy original_path)

```sql
SELECT original_path FROM trash_entries
WHERE trash_path LIKE ?;
```

Sau khi khôi phục:
```sql
DELETE FROM trash_entries WHERE trash_path LIKE ?;
```

### Xóa vĩnh viễn tất cả

```sql
DELETE FROM trash_entries;
```

### Đếm số file trong trash

```sql
SELECT COUNT(*) FROM trash_entries;
```

### Tính tổng dung lượng

```sql
SELECT SUM(size) FROM trash_entries;
```

## 2.5. Code example (C++ SQLite3 API)

### Khởi tạo database

```cpp
#include <sqlite3.h>

bool init_database(sqlite3*& db) {
    std::string trash_base = get_trash_dir(); // ~/.local/share/trash
    std::string db_path = trash_base + "/trash.db";
    
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) return false;

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS trash_entries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            original_path TEXT NOT NULL,
            trash_path TEXT NOT NULL,
            deleted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            size INTEGER DEFAULT 0,
            file_type TEXT DEFAULT 'file'
        );
        
        CREATE TABLE IF NOT EXISTS trash_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_dir TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        
        CREATE INDEX IF NOT EXISTS idx_original_path ON trash_entries(original_path);
        CREATE INDEX IF NOT EXISTS idx_deleted_at ON trash_entries(deleted_at);
        CREATE INDEX IF NOT EXISTS idx_session_dir ON trash_sessions(session_dir);
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
```

### Thêm file (prepared statement)

```cpp
bool add_entry(sqlite3* db, const std::string& name,
               const std::string& orig, const std::string& trash,
               int size, const std::string& type) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO trash_entries "
                      "(name, original_path, trash_path, deleted_at, size, file_type) "
                      "VALUES (?, ?, ?, ?, ?, ?)";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, orig.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, trash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, get_current_time_str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 5, size);
    sqlite3_bind_text(stmt, 6, type.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
```

### Truy vấn danh sách

```cpp
void list_entries(sqlite3* db) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name, original_path, trash_path, deleted_at, size, file_type "
                      "FROM trash_entries ORDER BY deleted_at DESC";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = (const char*)sqlite3_column_text(stmt, 0);
        const char* orig = (const char*)sqlite3_column_text(stmt, 1);
        const char* trash = (const char*)sqlite3_column_text(stmt, 2);
        const char* date = (const char*)sqlite3_column_text(stmt, 3);
        sqlite3_int64 size = sqlite3_column_int64(stmt, 4);
        const char* type = (const char*)sqlite3_column_text(stmt, 5);

        std::cout << name << " from " << orig << " (" << date << ") " << size << " bytes\n";
    }
    sqlite3_finalize(stmt);
}
```