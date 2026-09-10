# 04. Công nghệ nguồn mở được sử dụng

## 1. SQLite3

- **Phiên bản:** 3.45
- **Giấy phép:** Public Domain
- **Website:** https://www.sqlite.org
- **Mục đích:** Lưu trữ metadata về các file đã xóa (embedded database)
- **Lý do chọn:**
  - Nhẹ, không cần cấu hình server
  - Hỗ trợ trực tiếp qua API C++
  - Phổ biến trong các dự án nhỏ và trung bình
  - Hỗ trợ toàn bộ truy vấn SQL chuẩn
  - ACID transactions đảm bảo tính toàn vẹn
  - Source code đơn file (amalgamation) dễ nhúng

## 2. CMake

- **Phiên bản:** 3.14+
- **Giấy phép:** BSD
- **Website:** https://cmake.org
- **Mục đích:** Hệ thống build cho dự án
- **Lý do chọn:**
  - Chuẩn trong cộng đồng C++
  - Hỗ trợ đa nền tảng
  - Dễ cấu hình và mở rộng
  - Tích hợp với IDE và công cụ kiểm thử
  - Hỗ trợ install, packaging (CPack)

## 3. Google Test (hoặc framework tự viết)

- **Phiên bản:** 1.14 (nếu dùng) / Tự viết
- **Giấy phép:** BSD / MIT
- **Website:** https://github.com/google/googletest
- **Mục đích:** Kiểm thử đơn vị
- **Lý do chọn:**
  - Framework kiểm thử phổ biến nhất cho C++
  - Hỗ trợ assertion và mock
  - Dễ tích hợp với CMake
  - Tài liệu đầy đủ

## 4. POSIX API

- **Phiên bản:** - (phần mềm gốc của Linux)
- **Giấy phép:** - (part of the operating system)
- **Mục đích:** Thao tác file trên hệ thống Linux
- **Lý do chọn:**
  - API chuẩn cho Linux
  - Hỗ trợ đầy đủ các thao tác file
  - Hiệu suất cao
  - Phù hợp với yêu cầu dự án

## 5. Git

- **Phiên bản:** 2.40+
- **Giấy phép:** GPL
- **Website:** https://git-scm.com
- **Mục đích:** Quản lý phiên bản mã nguồn
- **Lý do chọn:**
  - Công cụ quản lý phiên bản phổ biến nhất
  - Hỗ trợ nhóm phát triển
  - Tích hợp với GitHub
  - Miễn phí, nguồn mở

## 6. GitHub

- **Website:** https://github.com
- **Mục đích:** Lưu trữ và quản lý mã nguồn
- **Lý do chọn:**
  - Cộng đồng lớn nhất cho nguồn mở
  - Hỗ trợ Issues, Projects, Actions
  - Miễn phí cho dự án public
  - Dễ hợp tác nhóm

## 7. nlohmann/json (đã loại bỏ)

**Lưu ý:** Phiên bản trước sử dụng nlohmann/json để lưu metadata dạng JSON file. Phiên bản hiện tại (v1.0.0) đã chuyển sang **SQLite3** hoàn toàn vì:
- Hiệu năng truy vấn tốt hơn cho danh sách lớn
- Hỗ trợ index, tìm kiếm nhanh
- Transaction đảm bảo tính nhất quán
- Không cần parse JSON thủ công (giảm bug parse)
- Giảm phụ thuộc (header-only vẫn cần include)

## Tóm tắt

| Công nghệ | Phiên bản | Giấy phép | Mục đích chính | Trạng thái |
|-----------|-----------|-----------|----------------|------------|
| SQLite3 | 3.45 | Public Domain | Lưu trữ metadata | Su dung |
| CMake | 3.14+ | BSD | He thong build | Su dung |
| Google Test | 1.14 | BSD | Kiem thu don vi | Su dung |
| POSIX API | - | - | Thao tac file | Su dung |
| Git | 2.40+ | GPL | Quan ly phien ban | Su dung |
| GitHub | - | - | Luu tru ma nguon | Su dung |
| nlohmann/json | 3.11 | MIT | Xu ly JSON | Loai bo |