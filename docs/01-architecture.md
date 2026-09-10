# 01. Kiến trúc hệ thống

## 1.1. Tổng quan

tr-cli là công cụ dòng lệnh cho phép người dùng xóa, khôi phục và quản lý file trên hệ thống Linux. Hệ thống được thiết kế đơn giản, dễ mở rộng và phù hợp với môi trường dòng lệnh.

## 1.2. Cấu trúc thư mục

```
tr-cli/
├── CMakeLists.txt          # Hệ thống build + cài đặt
├── README.md               # Hướng dẫn sử dụng
├── SPEC.MD                 # Giải thích chức năng
├── LICENSE                 # Giấy phép MIT
├── include/
│   └── trashcli.h          # Header chính
├── src/
│   ├── main.cpp            # Điểm vào chương trình + CLI parser
│   └── trash.cpp           # Xử lý nghiệp vụ + SQLite3
├── test/
│   └── test_trash.cpp      # Đơn vị kiểm thử
├── docs/
│   ├── 01-architecture.md  # Tài liệu này
│   ├── 02-database.md      # Hệ thống SQL
│   ├── 03-modules.md       # Các module
│   ├── 04-opensource.md    # Công nghệ nguồn mở
│   ├── backlog.md          # Product backlog
│   ├── sprint-log.md       # Nhật ký sprint
│   └── process.md          # Quy trình phát triển
├── thirdparty/
│   └── sqlite/
│       ├── sqlite3.h       # Header SQLite3
│       ├── sqlite3ext.h    # Header SQLite3 extension
│       ├── sqlite3.c       # Amalgamation source
│       └── sqlite.zip      # Nén SQLite3
├── scripts/
│   └── bugs/               # Script mô phỏng lỗi
│       ├── B01_metadata.sh
│       ├── B02_crash.sh
│       ├── B03_permission.sh
│       ├── B04_duplicate.sh
│       └── B05_relative_path.sh
└── build/                  # Thư mục build (bỏ qua git)
```

## 1.3. Luồng xử lý

Luồng xử lý chính của hệ thống:

```
Người dùng
    │
    ▼
┌─────────────┐
│  CLI Parser │  Phân tích tham số dòng lệnh
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Command    │  Chọn chức năng phù hợp
│  Dispatcher │
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────────┐
│              Business Logic             │
├─────┬─────┬─────┬─────┬─────┬──────────┤
│trash│list │info │restore│empty│permanent│
│     │     │     │      │     │_delete   │
└──┬──┴──┬──┴──┬──┴──┬───┴──┬──┴─────────┘
   │     │     │     │      │
   ▼     ▼     ▼     ▼      ▼
┌─────────────────────────────────────────┐
│           Data Access Layer             │
│     SQLite3 (embedded database)         │
│     trash_entries, trash_sessions       │
└─────────────────────────────────────────┘
   │
   ▼
┌─────────────────────────────────────────┐
│           File System Layer             │
│     std::filesystem (POSIX API)         │
└─────────────────────────────────────────┘
```

## 1.4. Cấu trúc thư mục trash

```
~/.local/share/trash/
├── trash.db                 # SQLite3 database
├── 20260910-205150-2b42b/
│   └── data/
│       └── testfile.txt    # File đã xóa
├── 20260910-205230-abc12/
│   └── data/
│       └── mydir/          # Thư mục đã xóa
│           ├── a.txt
│           └── b.txt
```

Định dạng tên thư mục session: `YYYYMMDD-HHmmss-5hex`

Cơ sở dữ liệu `trash.db` chứa 2 bảng:
- `trash_entries`: Metadata từng file/thư mục
- `trash_sessions`: Thông tin phiên xóa

## 1.5. Công nghệ sử dụng

| Công nghệ | Phiên bản | Mục đích |
|-----------|-----------|----------|
| C++ | 17 | Ngôn ngữ lập trình |
| CMake | 3.14+ | Hệ thống build |
| POSIX API | - | Thao tác file |
| SQLite3 | 3.45+ | Lưu trữ metadata (embedded) |
| std::filesystem | C++17 | Thao tác thư mục/file |

## 1.6. Thiết kế dữ liệu

### Bảng trash_entries
| Cột | Kiểu | Mô tả |
|-----|------|-------|
| id | INTEGER | Khóa chính, tự tăng |
| name | TEXT | Tên file/thư mục |
| original_path | TEXT | Đường dẫn gốc |
| trash_path | TEXT | Đường dẫn session |
| deleted_at | TIMESTAMP | Thời điểm xóa |
| size | INTEGER | Kích thước (bytes) |
| file_type | TEXT | 'file' hoặc 'directory' |

### Bảng trash_sessions
| Cột | Kiểu | Mô tả |
|-----|------|-------|
| id | INTEGER | Khóa chính, tự tăng |
| session_dir | TEXT | Tên thư mục session |
| created_at | TIMESTAMP | Thời điểm tạo |

Indexes: `idx_original_path`, `idx_deleted_at`, `idx_session_dir`