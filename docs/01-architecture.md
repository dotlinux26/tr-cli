# 01. Kiến trúc hệ thống

## 1.1. Tổng quan

tr-cli là công cụ dòng lệnh cho phép người dùng xóa, khôi phục và quản lý file trên hệ thống Linux. Hệ thống được thiết kế đơn giản, dễ mở rộng và phù hợp với môi trường dòng lệnh.

## 1.2. Cấu trúc thư mục

```
tr-cli/
├── CMakeLists.txt          # Hệ thống build
├── README.md               # Hướng dẫn sử dụng
├── SPEC.MD                 # Giải thích chức năng
├── LICENSE                 # Giấy phép MIT
├── include/
│   └── trashcli.h          # Header chính
├── src/
│   ├── main.cpp            # Điểm vào chương trình
│   └── trash.cpp           # Hiện thị chức năng
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
│   ├── json/
│   │   └── json.hpp        # Thư viện nlohmann/json
│   └── sqlite/
│       ├── sqlite3.h       # Header SQLite3
│       └── sqlite3ext.h    # Header SQLite3 extension
├── scripts/
│   └── bugs/               # Script mô phỏng lỗi
│       ├── B01_metadata.sh
│       ├── B02_crash.sh
│       ├── B03_permission.sh
│       ├── B04_duplicate.sh
│       └── B05_relative_path.sh
└── build/                  # Thư mục build
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
│           File System Layer             │
│     std::filesystem (POSIX API)         │
└─────────────────────────────────────────┘
```

## 1.4. Cấu trúc thư mục trash

```
~/.local/share/trash/
├── 20260910-205150-2b42b/
│   ├── metadata.json       # Thông tin file gốc
│   └── data/
│       └── testfile.txt    # File đã xóa
├── 20260910-205230-abc12/
│   ├── metadata.json
│   └── data/
│       └── mydir/          # Thư mục đã xóa
│           ├── a.txt
│           └── b.txt
```

Định dạng tên thư mục: `YYYYMMDD-HHmmss-5hex`

Nội dung metadata.json:
```json
{
  "name": "testfile.txt",
  "original_path": "/home/user/testfile.txt",
  "deleted_at": "2026-09-10 20:51:50",
  "size": 1024
}
```

## 1.5. Công nghệ sử dụng

| Công nghệ | Phiên bản | Mục đích |
|-----------|-----------|----------|
| C++ | 17 | Ngôn ngữ lập trình |
| CMake | 3.14+ | Hệ thống build |
| POSIX API | - | Thao tác file |
| SQLite3 | 3.45+ | Lưu trữ metadata (kế hoạch) |
| nlohmann/json | 3.11+ | Xử lý JSON |
| Google Test | 1.14+ | Kiểm thử đơn vị |
