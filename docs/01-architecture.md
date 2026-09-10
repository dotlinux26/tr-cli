# 01. Tổng quan về dự án

## 1.1. Hiến chương dự án

### 1.1.1. Xác định dự án

| Thông tin | Nội dung |
|-----------|----------|
| Tên dự án | Trash CLI (tr) |
| Mô tả | Công cụ dòng lệnh thay thế `rm`, chuyển file vào thùng rác thay vì xóa vĩnh viễn. Hỗ trợ khôi phục, liệt kê, xem thông tin và xóa vĩnh viễn khi cần. |
| Nhà tài trợ | Trường Đại học Công nghiệp Hà Nội (HaUI) |
| Quản lý dự án | Nguyễn Đức Cảnh |
| Nguồn lực đội dự án | 3 thành viên: Nguyễn Đức Cảnh (PO/SM/Developer), Lò Thanh Tùng (Technical Writer), Nguyễn Khắc Nam Khánh (Technical Writer) |

### 1.1.2. Phạm vi, mục đích và mục tiêu của dự án

#### 1.1.2.1. Phạm vi
**Thuộc dự án:**
- Lệnh `tr <file>`: Xóa file vào thùng rác
- Lệnh `tr -r <dir>`: Xóa đệ quy thư mục
- Lệnh `tr -rf <file>`: Xóa vĩnh viễn
- Lệnh `tr -rf *` / `tr -rf <dir>/*`: Xóa nội dung, giữ thư mục
- Lệnh `tr -re <ID>`: Khôi phục theo ID
- Lệnh `tr -ref <ID>`: Khôi phục ghi đè
- Lệnh `tr -l`: Liệt kê file trong thùng rác (bảng căn chỉnh cột)
- Lệnh `tr -i <ID>` / `tr -info <ID>`: Xem thông tin
- Lệnh `tr -e`: Xóa toàn bộ thùng rác
- Metadata lưu trữ bằng SQLite3 embedded database

**Không thuộc dự án:**
- Giao diện đồ họa (GUI)
- Đồng bộ thùng rác qua mạng
- Tích hợp desktop environment (KDE/GNOME trash spec)

#### 1.1.2.2. Mục đích
Phát triển công cụ dòng lệnh an toàn thay thế `rm`, giúp người dùng Linux xóa file mà không lo mất dữ liệu vĩnh viễn.

#### 1.1.2.3. Mục tiêu
| Mục tiêu | Mô tả chi tiết |
|----------|----------------|
| MT1 | Xây dựng core engine xóa/khôi phục file an toàn (trash/restore) |
| MT2 | Triển khai SQLite3 cho metadata, khắc phục bug parse JSON |
| MT3 | Hiển thị bảng `tr -l` căn chỉnh cột chính xác (printf) |
| MT4 | Hỗ trợ khôi phục thư mục không crash (query file_type) |
| MT5 | Hoàn thành 10 User Story, tổng 30 story points |
| MT6 | Đóng gói release v1.0.0 (.deb, .tar.gz, .zip) + GitHub Release |

## 1.2. Môi trường và công cụ hỗ trợ phát triển

### 1.2.1. Môi trường phát triển
- **IDE:** VS Code / Vim
- **Quản lý mã nguồn:** Git + GitHub
- **Build system:** CMake 3.14+
- **Compiler:** GCC 9+ / Clang 10+ (C++17)
- **OS mục tiêu:** Linux (POSIX)

### 1.2.2. Công cụ phát triển
| Danh mục | Công cụ | Mô tả |
|----------|---------|-------|
| Ngôn ngữ | C++17 | Core logic |
| Build | CMake | Cross-platform build |
| Database | SQLite3 3.45+ | Embedded metadata storage |
| Filesystem | std::filesystem (C++17) | Thao tác file/thư mục |
| Test | CTest + custom | Unit test |
| Package | dpkg-deb, tar, zip | Release packaging |

## Kết luận chương 1
Chương 1 đã trình bày hiến chương dự án Trash CLI: xác định dự án, phạm vi, mục tiêu, môi trường và công cụ phát triển. Các yêu cầu chức năng được định nghĩa rõ ràng qua 10 User Story. Chương 2 sẽ trình bày chi tiết quy trình lập kế hoạch và thực hiện dự án theo phương pháp Scrum.