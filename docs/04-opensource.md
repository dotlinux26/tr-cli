# 04. Công nghệ nguồn mở được sử dụng

## 1. SQLite3

- **Phiên bản:** 3.45
- **Giấy phép:** Public Domain
- **Website:** https://www.sqlite.org
- **Mục đích:** Lưu trữ metadata về các file đã xóa
- **Lý do chọn:**
  - Nhẹ, không cần cấu hình server
  - Hỗ trợ trực tiếp qua API C++
  - Phổ biến trong các dự án nhỏ và trung bình
  - Hỗ trợ toàn bộ truy vấn SQL chuẩn

## 2. nlohmann/json

- **Phiên bản:** 3.11
- **Giấy phép:** MIT
- **Website:** https://github.com/nlohmann/json
- **Mục đích:** Đọc và ghi file metadata.json
- **Lý do chọn:**
  - Header-only, dễ tích hợp
  - API đơn giản, dễ sử dụng
  - Hỗ trợ parse JSON từ string và file
  - Phổ biến trong cộng đồng C++

## 3. CMake

- **Phiên bản:** 3.14+
- **Giấy phép:** BSD
- **Website:** https://cmake.org
- **Mục đích:** Hệ thống build cho dự án
- **Lý do chọn:**
  - Chuẩn trong cộng đồng C++
  - Hỗ trợ đa nền tảng
  - Dễ cấu hình và mở rộng
  - Tích hợp với IDE và công cụ kiểm thử

## 4. Google Test

- **Phiên bản:** 1.14
- **Giấy phép:** BSD
- **Website:** https://github.com/google/googletest
- **Mục đích:** Kiểm thử đơn vị
- **Lý do chọn:**
  - Framework kiểm thử phổ biến nhất cho C++
  - Hỗ trợ assertion và mock
  - Dễ tích hợp với CMake
  - Tài liệu đầy đủ

## 5. POSIX API

- **Phiên bản:** - (phần mềm gốc của Linux)
- **Giấy phép:** - (part of the operating system)
- **Mục đích:** Thao tác file trên hệ thống Linux
- **Lý do chọn:**
  - API chuẩn cho Linux
  - Hỗ trợ đầy đủ các thao tác file
  - Hiệu suất cao
  - Phù hợp với yêu cầu dự án

## 6. Git

- **Phiên bản:** 2.40+
- **Giấy phép:** GPL
- **Website:** https://git-scm.com
- **Mục đích:** Quản lý phiên bản mã nguồn
- **Lý do chọn:**
  - Công cụ quản lý phiên bản phổ biến nhất
  - Hỗ trợ nhóm phát triển
  - Tích hợp với GitHub
  - Miễn phí, nguồn mở

## 7. GitHub

- **Website:** https://github.com
- **Mục đích:** Lưu trữ và quản lý mã nguồn
- **Lý do chọn:**
  - Cộng đồng lớn nhất cho nguồn mở
  - Hỗ trợ Issues, Projects, Actions
  - Miễn phí cho dự án public
  - Dễ hợp tác nhóm

## Tóm tắt

| Công nghệ | Phiên bản | Giấy phép | Mục đích chính |
|-----------|-----------|-----------|----------------|
| SQLite3 | 3.45 | Public Domain | Lưu trữ metadata |
| nlohmann/json | 3.11 | MIT | Xử lý JSON |
| CMake | 3.14+ | BSD | Hệ thống build |
| Google Test | 1.14 | BSD | Kiểm thử đơn vị |
| POSIX API | - | - | Thao tác file |
| Git | 2.40+ | GPL | Quản lý phiên bản |
| GitHub | - | - | Lưu trữ mã nguồn |
