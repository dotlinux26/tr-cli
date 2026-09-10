# 04. Công nghệ nguồn mở được sử dụng

## 4.1. SQLite3

- **Phiên bản:** 3.45+
- **Giấy phép:** Public Domain
- **Website:** https://www.sqlite.org
- **Mục đích:** Embedded database lưu trữ metadata (trash_entries, trash_sessions)
- **Lý do chọn:**
  - Nhẹ, không cần cấu hình server, single file
  - Hỗ trợ trực tiếp qua API C/C++
  - ACID transactions, chuẩn SQL
  - Amalgamation source (sqlite3.c) dễ nhúng vào project
  - Phổ biến, ổn định, zero-config

## 4.2. CMake

- **Phiên bản:** 3.14+
- **Giấy phép:** BSD-3-Clause
- **Website:** https://cmake.org
- **Mục đích:** Cross-platform build system
- **Lý do chọn:**
  - Chuẩn de facto cho C++
  - Hỗ trợ multi-platform (Linux, Windows, macOS)
  - Tích hợp CTest, CPack
  - Dễ cấu hình install, packaging

## 4.3. Google Test / Custom Test Framework

- **Phiên bản:** 1.14 (Google Test) / Tự viết
- **Giấy phép:** BSD-3-Clause / MIT
- **Website:** https://github.com/google/googletest
- **Mục đích:** Unit testing
- **Lý do chọn:**
  - Framework phổ biến nhất cho C++
  - Hỗ trợ assertions, death tests, parameterized tests
  - Tích hợp CMake/CTest
  - Dễ mở rộng

## 4.4. POSIX API / std::filesystem (C++17)

- **Phiên bản:** C++17 standard / Linux kernel
- **Giấy phép:** ISO C++ / GPL (kernel)
- **Mục đích:** Thao tác file system (rename, remove, stat, iterate)
- **Lý do chọn:**
  - API chuẩn, portable trên POSIX
  - Hiệu suất cao (syscall trực tiếp)
  - Type-safe, exception-safe (C++17 filesystem)

## 4.5. Git

- **Phiên bản:** 2.40+
- **Giấy phép:** GPL-2.0
- **Website:** https://git-scm.com
- **Mục đích:** Version control
- **Lý do chọn:** Chuẩn công nghiệp, distributed, branching model mạnh

## 4.6. GitHub

- **Website:** https://github.com
- **Mục đích:** Source hosting, Issues, Releases, Actions
- **Lý do chọn:** Cộng đồng lớn nhất, miễn phí cho public repo, CI/CD tích hợp

## 4.7. Công nghệ đã loại bỏ

### nlohmann/json (v3.11, MIT)
- **Lý do loại bỏ:** Phiên bản đầu dùng JSON file cho metadata. Gặp Bug #6 (parse offset error khi đọc `original_path`). Chuyển sang SQLite3 hoàn toàn để:
  - Khắc phục bug parse
  - Hiệu năng truy vấn tốt hơn (index, ORDER BY)
  - Transaction đảm bảo tính nhất quán
  - Giảm dependency (SQLite3 đã có sẵn trên Linux)

## Tóm tắt

| Công nghệ | Phiên bản | Giấy phép | Mục đích | Trạng thái |
|-----------|-----------|-----------|----------|------------|
| SQLite3 | 3.45+ | Public Domain | Embedded DB metadata | Su dung |
| CMake | 3.14+ | BSD-3 | Build system | Su dung |
| Google Test | 1.14 | BSD-3 | Unit test | Su dung |
| std::filesystem | C++17 | ISO | File operations | Su dung |
| Git | 2.40+ | GPL-2 | Version control | Su dung |
| GitHub | - | - | Hosting, CI/CD | Su dung |
| nlohmann/json | 3.11 | MIT | JSON parsing | Loai bo |

## Kết luận chương 4
Chương 4 đã liệt kê toàn bộ công nghệ nguồn mở sử dụng trong Trash CLI. Điểm nổi bật là việc quyết định loại bỏ nlohmann/json và chuyển hoàn toàn sang SQLite3 sau khi phát hiện Bug #6, thể hiện tư duy thực dụng: chọn công nghệ phù hợp nhất cho bài toán thay vì theo trend.