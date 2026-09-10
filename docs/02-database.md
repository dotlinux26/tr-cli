# 02. Hệ thống SQL (SQLite3)

## 2.1. Giới thiệu
Hệ thống sử dụng **SQLite3** (embedded database) để lưu trữ metadata về các file đã xóa. SQLite3 được chọn vì tính nhẹ, không cần cấu hình server, hỗ trợ trực tiếp qua API C++, ACID transactions đảm bảo tính toàn vẹn dữ liệu.

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
| name | TEXT | Tên file/thư mục gốc |
| original_path | TEXT | Đường dẫn đầy đủ của file/thư mục gốc |
| trash_path | TEXT | Đường dẫn thư mục session trong trash |
| deleted_at | TIMESTAMP | Thời điểm xóa |
| size | INTEGER | Kích thước (bytes) |
| file_type | TEXT | Loại: 'file' hoặc 'directory' |

### Bảng: trash_sessions

```sql
CREATE TABLE trash_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_dir TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

Mô tả:
| Cột | Kiểu dữ liệu | Mô tả |
|-----|-------------|-------|
| id | INTEGER | Mã định danh, tự tăng |
| session_dir | TEXT | Tên thư mục session (format: YYYYMMDD-HHMMSS-5hex) |
| created_at | TIMESTAMP | Thời điểm tạo session |

## 2.3. Cấu trúc index

```sql
CREATE INDEX idx_original_path ON trash_entries(original_path);
CREATE INDEX idx_deleted_at ON trash_entries(deleted_at);
CREATE INDEX idx_session_dir ON trash_sessions(session_dir);
```

## 2.4. Các truy vấn thường dùng

### 2.4.1. Thêm file mới vào trash
```sql
INSERT INTO trash_entries (name, original_path, trash_path, deleted_at, size, file_type)
VALUES (?, ?, ?, ?, ?, ?);
```

### 2.4.2. Liệt kê tất cả file trong trash
```sql
SELECT id, name, original_path, trash_path, deleted_at, size, file_type
FROM trash_entries
ORDER BY deleted_at DESC;
```

### 2.4.3. Tìm kiếm file theo tên
```sql
SELECT * FROM trash_entries
WHERE name LIKE ?
ORDER BY deleted_at DESC;
```

### 2.4.4. Lấy thông tin file theo session ID
```sql
SELECT original_path, file_type FROM trash_entries
WHERE trash_path LIKE ?;
```

### 2.4.5. Khôi phục file (xóa record sau khi restore)
```sql
DELETE FROM trash_entries WHERE trash_path LIKE ?;
```

### 2.4.6. Xóa vĩnh viễn tất cả
```sql
DELETE FROM trash_entries;
```

### 2.4.7. Đếm số file trong trash
```sql
SELECT COUNT(*) FROM trash_entries;
```

### 2.4.8. Tính tổng dung lượng
```sql
SELECT SUM(size) FROM trash_entries;
```

## 2.5. Code example (C++ SQLite3 API)

### 2.5.1. Khởi tạo database
```cpp
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

### 2.5.2. Thêm file (prepared statement)
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

### 2.5.3. Truy vấn danh sách
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

### 2.5.4. Khôi phục file - query file_type
```cpp
bool restore_file(const std::string& trash_id, bool force) {
    // Query thêm file_type để phân biệt file vs directory
    const char* sql = "SELECT original_path, file_type FROM trash_entries WHERE trash_path LIKE ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    std::string pattern = "%" + trash_id + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    
    std::string original, file_type;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        original = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        file_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);
    
    if (file_type == "directory") {
        // Restore thư mục: rename subdirectory trong data/ về original_path
    } else {
        // Restore file: rename file trong data/ về original_path
    }
    // DELETE FROM trash_entries WHERE trash_path LIKE ?
}
```

## Kết luận chương 2
Chương 2 đã trình bày chi tiết cấu trúc database SQLite3 cho Trash CLI: 2 bảng chính (trash_entries, trash_sessions), các index, toàn bộ truy vấn CRUD cần thiết, và code example C++ sử dụng prepared statements. Việc chuyển từ JSON sang SQLite3 đã khắc phục hoàn toàn Bug #6 (parse offset error) và tăng hiệu năng truy vấn.