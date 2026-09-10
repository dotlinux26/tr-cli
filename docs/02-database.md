# 02. Hệ thống SQL

## 2.1. Giới thiệu

Hệ thống sử dụng SQLite3 để lưu trữ metadata về các file đã xóa. SQLite3 được chọn vì tính nhẹ, không cần cấu hình server và hỗ trợ trực tiếp qua API C++.

## 2.2. Cấu trúc bảng

### Bảng: trash_entries

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
| trash_path | TEXT | Đường dẫn file trong trash |
| deleted_at | TIMESTAMP | Thời điểm xóa |
| size | INTEGER | Kích thước file (byte) |
| file_type | TEXT | Loại file (file/dir) |

### Bảng: trash_sessions

```sql
CREATE TABLE trash_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_dir TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

## 2.3. Cấu trúc index

```sql
CREATE INDEX idx_original_path ON trash_entries(original_path);
CREATE INDEX idx_deleted_at ON trash_entries(deleted_at);
CREATE INDEX idx_session_dir ON trash_sessions(session_dir);
```

## 2.4. Các truy vấn thường dùng

### Thêm file mới vào trash

```sql
INSERT INTO trash_entries (name, original_path, trash_path, size, file_type)
VALUES (?, ?, ?, ?, ?);
```

### Liệt kê tất cả file trong trash

```sql
SELECT id, name, original_path, deleted_at, size
FROM trash_entries
ORDER BY deleted_at DESC;
```

### Tìm kiếm file theo tên

```sql
SELECT * FROM trash_entries
WHERE name LIKE ?
ORDER BY deleted_at DESC;
```

### Lấy thông tin file

```sql
SELECT * FROM trash_entries
WHERE id = ?;
```

### Khôi phục file

```sql
SELECT original_path FROM trash_entries
WHERE id = ?;
```

Sau khi khôi phục:
```sql
DELETE FROM trash_entries WHERE id = ?;
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

## 2.5. Code example

### Khởi tạo database

```cpp
#include <sqlite3.h>

bool init_database(sqlite3*& db) {
    int rc = sqlite3_open("trash.db", &db);
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

### Thêm file

```cpp
bool add_entry(sqlite3* db, const std::string& name,
               const std::string& orig, const std::string& trash,
               int size) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO trash_entries "
                      "(name, original_path, trash_path, size) "
                      "VALUES (?, ?, ?, ?)";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, orig.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, trash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, size);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
```

### Truy vấn danh sách

```cpp
void list_entries(sqlite3* db) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, name, original_path, deleted_at "
                      "FROM trash_entries ORDER BY deleted_at DESC";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* orig = (const char*)sqlite3_column_text(stmt, 2);
        const char* date = (const char*)sqlite3_column_text(stmt, 3);

        std::cout << "#" << id << " " << name
                  << " from " << orig
                  << " (" << date << ")\n";
    }
    sqlite3_finalize(stmt);
}
```
