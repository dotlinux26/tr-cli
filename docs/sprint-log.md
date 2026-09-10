# 07. Nhật ký Sprint

## 7.1. Phân bổ câu chuyện người dùng cho các chu trình phát triển phần mềm

Mỗi Sprint thực hiện tối đa không quá 4 tuần. Điểm story point và ước tính Effort cho các Sprint phải tương đối cân bằng nhau.

Bảng 7.1. Phân bổ User Story cho các Sprint

| UID | User Story | Story Point | Estimation Effort | Sprint | Status |
|-----|------------|-------------|-------------------|--------|--------|
| US-001 | Xóa file vào thùng rác (`tr`) | 3 | 4 giờ | Sprint 1 (V0) | Done |
| US-007 | Liệt kê file trong trash (`-l`) | 2 | 3 giờ | Sprint 1 (V0) | Done |
| US-009 | Xóa vĩnh viễn tất cả file (`-e`) | 2 | 2 giờ | Sprint 1 (V0) | Done |
| US-004 | Xóa đệ quy (`-r`) | 5 | 8 giờ | Sprint 2 (V1.0) | Done |
| US-005 | Khôi phục file (`-re`) | 5 | 8 giờ | Sprint 2 (V1.0) | Done |
| US-008 | Xem thông tin file (`-i`) | 2 | 3 giờ | Sprint 2 (V1.0) | Done |
| US-006 | Khôi phục ghi đè (`-ref`) | 3 | 4 giờ | Sprint 3 (V2.0) | Done |
| US-002 | Xóa file vĩnh viễn (`-rf`) | 2 | 2 giờ | Sprint 3 (V2.0) | Done |
| US-003 | Xóa nội dung giữ thư mục (`-rf *`) | 3 | 4 giờ | Sprint 3 (V2.0) | Done |
| US-010 | Hiển thị thông tin chi tiết (`-info`) | 3 | 4 giờ | Sprint 4 (V3.0) | Done |

**Tổng story point:** 30 | **Tổng estimation effort:** 42 giờ

| Sprint | Phiên bản | Story Point | Effort | Thời gian |
|--------|-----------|-------------|--------|-----------|
| Sprint 1 | V0 | 7 | 9 giờ | 10/09 - 24/09 |
| Sprint 2 | V1.0 | 12 | 19 giờ | 25/09 - 08/10 |
| Sprint 3 | V2.0 | 8 | 10 giờ | 09/10 - 22/10 |
| Sprint 4 | V3.0 | 3 | 4 giờ | 23/10 - 05/11 |

---

## 7.2. Danh sách các câu chuyện người dùng trong các chu trình phát triển phần mềm

### Sprint Backlog - Sprint 1 (V0) | 10/09/2026 - 24/09/2026

Bảng 7.2. Sprint Backlog Sprint 1

| UID | User Story | Story Point | Estimation Effort | Status | Acceptance Criteria | Testing | Task owner | Task | Subtask |
|-----|------------|-------------|-------------------|--------|--------------------|---------|------------|------|---------|
| US-001 | Xóa file vào thùng rác (`tr`) | 3 | 4 giờ | Done | File di chuyển đến trash; metadata.json được tạo; File gốc biến mất | Chạy `tr test.txt` -> kiểm tra file trong trash | Nguyễn Đức Cảnh | Triển khai hàm trash() | Tạo thư mục theo định dạng YYYYMMDD-HHmmss-5hex; Di chuyển file bằng rename(); Ghi metadata.json |
| US-007 | Liệt kê file trong trash (`-l`) | 2 | 3 giờ | Done | Hiển thị tên file, đường dẫn gốc, ngày xóa; Sắp xếp mới nhất | Chạy `tr -l` -> kiểm tra danh sách | Nguyễn Đức Cảnh | Triển khai hàm list() | Đọc thư mục trash; Parse metadata.json; Hiển thị dạng bảng |
| US-009 | Xóa vĩnh viễn tất cả file (`-e`) | 2 | 2 giờ | Done | Tất cả file bị xóa; Trash gốc giữ lại; Xác nhận trước khi xóa | Chạy `tr -e` -> kiểm tra trash trống | Nguyễn Đức Cảnh | Triển khai hàm empty() | Duyệt thư mục trash; Xóa từng file bằng unlink(); Giữ thư mục gốc |
| - | Viết tài liệu Sprint 1 | - | 3 giờ | Done | Tài liệu quy trình, báo cáo Sprint 1 hoàn thành | Kiểm tra tài liệu | Lò Thanh Tùng, Nguyễn Khắc Nam Khánh | Viết báo cáo | Nhận xét Sprint 1; Viết báo cáo quy trình |

### Sprint Backlog - Sprint 2 (V1.0) | 25/09/2026 - 08/10/2026

Bảng 7.3. Sprint Backlog Sprint 2

| UID | User Story | Story Point | Estimation Effort | Status | Acceptance Criteria | Testing | Task owner | Task | Subtask |
|-----|------------|-------------|-------------------|--------|--------------------|---------|------------|------|---------|
| US-004 | Xóa đệ quy (`-r`) | 5 | 8 giờ | Done | Tất cả file/thư mục con di chuyển vào trash; Cấu trúc bảo toàn; metadata ghi đường dẫn gốc | Tạo thư mục cấp 2 -> `tr -r testdir` -> kiểm tra trong trash | Nguyễn Đức Cảnh | Triển khai hàm trash_recursive() | Duyệt đệ quy thư mục; Di chuyển từng file; Giữ cấu trúc thư mục trong trash |
| US-005 | Khôi phục file (`-re`) | 5 | 8 giờ | Done | File di chuyển về vị trí gốc; File xóa khỏi trash; Cảnh báo nếu file đã tồn tại | Trash file -> `tr -re <path>` -> kiểm tra file về gốc | Nguyễn Đức Cảnh | Triển khai hàm restore() | Đọc metadata.json; Di chuyển file về đường dẫn gốc; Xóa thư mục trash |
| US-008 | Xem thông tin file (`-i`) | 2 | 3 giờ | Done | Hiển thị tên, kích thước, đường dẫn gốc, ngày xóa, loại file | Trash file -> `tr -i <file>` -> kiểm tra thông tin | Nguyễn Đức Cảnh | Triển khai hàm info() | Đọc metadata.json; Hiển thị thông tin dạng bảng |
| - | Viết tài liệu Sprint 2 | - | 3 giờ | Done | Tài liệu quy trình, báo cáo Sprint 2 hoàn thành | Kiểm tra tài liệu | Lò Thanh Tùng, Nguyễn Khắc Nam Khánh | Viết báo cáo | Nhận xét Sprint 2; Viết báo cáo quy trình |

### Sprint Backlog - Sprint 3 (V2.0) | 09/10/2026 - 22/10/2026

Bảng 7.4. Sprint Backlog Sprint 3

| UID | User Story | Story Point | Estimation Effort | Status | Acceptance Criteria | Testing | Task owner | Task | Subtask |
|-----|------------|-------------|-------------------|--------|--------------------|---------|------------|------|---------|
| US-006 | Khôi phục ghi đè (`-ref`) | 3 | 4 giờ | Done | File ghi đè tại vị trí gốc; File xóa khỏi trash | Tạo file trùng tên -> `tr -ref <path>` -> kiểm tra ghi đè | Nguyễn Đức Cảnh | Triển khai hàm restore_force() | Gọi restore() với cờ force; Ghi đè file bằng rename() |
| US-002 | Xóa file vĩnh viễn (`-rf`) | 2 | 2 giờ | Done | File xóa vĩnh viễn; Không có bản sao; Mã lỗi nếu file không tồn tại | `tr -rf testfile.txt` -> kiểm tra file biến mất hoàn toàn | Nguyễn Đức Cảnh | Triển khai hàm permanent_delete() | Xóa file bằng unlink() trực tiếp; Không di chuyển vào trash |
| US-003 | Xóa nội dung giữ thư mục (`-rf *`) | 3 | 4 giờ | Done | Tất cả file/thư mục con bị xóa; Thư mục gốc rỗng | Tạo thư mục có nội dung -> `tr -rf *` -> kiểm tra rỗng | Nguyễn Đức Cảnh | Triển khai hàm empty_keep_dir() | Duyệt nội dung thư mục; Xóa từng file/thư mục; Giữ thư mục gốc |
| - | Viết tài liệu Sprint 3 | - | 3 giờ | Done | Tài liệu quy trình, báo cáo Sprint 3 hoàn thành | Kiểm tra tài liệu | Lò Thanh Tùng, Nguyễn Khắc Nam Khánh | Viết báo cáo | Nhận xét Sprint 3; Viết báo cáo quy trình |

### Sprint Backlog - Sprint 4 (V3.0) | 23/10/2026 - 05/11/2026

Bảng 7.5. Sprint Backlog Sprint 4

| UID | User Story | Story Point | Estimation Effort | Status | Acceptance Criteria | Testing | Task owner | Task | Subtask |
|-----|------------|-------------|-------------------|--------|--------------------|---------|------------|------|---------|
| US-010 | Hiển thị thông tin chi tiết (`-info`) | 3 | 4 giờ | Done | Hiển thị tên, kích thước (byte/KB/MB), đường dẫn gốc, ngày xóa, loại file, quyền truy cập; Hỗ trợ thư mục | `tr -info <file>` -> kiểm tra đầy đủ thông tin | Nguyễn Đức Cảnh | Triển khai hàm info_detail() | Đọc metadata.json; Lấy thông tin stat(); Format kích thước; Hiển thị quyền truy cập |
| - | Kiểm thử tích hợp | - | 4 giờ | Done | Tất cả chức năng hoạt động đồng bộ | Chạy toàn bộ test case | Nguyễn Đức Cảnh | Kiểm thử tích hợp | Chạy unit test; Kiểm thử thủ công; Sửa lỗi |
| - | Hoàn thiện tài liệu | - | 4 giờ | Done | README.md, SPEC.MD, docs/ đầy đủ | Kiểm tra tài liệu | Lò Thanh Tùng, Nguyễn Khắc Nam Khánh | Viết tài liệu | Cập nhật README.md; Hoàn thiện báo cáo; Tổng hợp tài liệu |

---

## 7.3. Phát triển phiên bản sản phẩm phần mềm theo Sprint

### 7.3.1. Phiên bản phần mềm V0 (Sprint 1)

**Thời gian:** 10/09/2026 - 24/09/2026

**Mục tiêu:** Thiết lập cấu trúc dự án, triển khai chức năng cơ bản (xóa, liệt kê, xóa vĩnh viễn)

**Kế hoạch thực hiện:**
- Ngày 1-3: Thiết lập CMakeLists.txt, cấu trúc thư mục, cài đặt thư viện
- Ngày 4-8: Triển khai hàm `trash()`, `list()`, `empty()`
- Ngày 9-11: Viết unit test
- Ngày 12-14: Kiểm thử, fix lỗi, commit

**Các tính năng hoàn thành:**
- Lệnh `tr <file>`: Xóa file vào thùng rác
- Lệnh `tr -l`: Liệt kê file trong thùng rác
- Lệnh `tr -e`: Xóa vĩnh viễn toàn bộ thùng rác

**Đánh giá:** Dự án có cấu trúc ổn định. Các chức năng cơ bản hoạt động đúng.

### 7.3.2. Phiên bản phần mềm V1.0 (Sprint 2)

**Thời gian:** 25/09/2026 - 08/10/2026

**Mục tiêu:** Triển khai chức năng nâng cao (xóa đệ quy, khôi phục, xem thông tin)

**Kế hoạch thực hiện:**
- Ngày 1-4: Triển khai hàm `trash_recursive()` và `restore()`
- Ngày 5-7: Triển khai hàm `info()`
- Ngày 8-10: Viết unit test
- Ngày 11-14: Kiểm thử, fix lỗi, commit

**Các tính năng hoàn thành:**
- Lệnh `tr -r <dir>`: Xóa đệ quy thư mục
- Lệnh `tr -re <path>`: Khôi phục file
- Lệnh `tr -i <file>`: Xem thông tin file

**Đánh giá:** Chức năng nâng cao hoạt động ổn định. Cần xử lý tốt hơn trường hợp ngoại lệ.

### 7.3.3. Phiên bản phần mềm V2.0 (Sprint 3)

**Thời gian:** 09/10/2026 - 22/10/2026

**Mục tiêu:** Hoàn thiện tất cả chức năng xóa và khôi phục

**Kế hoạch thực hiện:**
- Ngày 1-3: Triển khai `restore_force()`, `permanent_delete()`
- Ngày 4-6: Triển khai `empty_keep_dir()`
- Ngày 7-10: Viết unit test
- Ngày 11-14: Kiểm thử, fix lỗi, commit

**Các tính năng hoàn thành:**
- Lệnh `tr -ref <path>`: Khôi phục ghi đè
- Lệnh `tr -rf <file>`: Xóa vĩnh viễn
- Lệnh `tr -rf *`: Xóa nội dung giữ thư mục

**Đánh giá:** Tất cả chức năng xóa/khôi phục đã hoàn thành. Cần tối ưu hiệu năng.

### 7.3.4. Phiên bản phần mềm V3.0 (Sprint 4) - PHÁT HÀNH 1.0.0

**Thời gian:** 23/10/2026 - 05/11/2026

**Mục tiêu:** Hoàn thiện, kiểm thử toàn diện, phát hành phiên bản 1.0.0

**Kế hoạch thực hiện:**
- Ngày 1-4: Triển khai `info_detail()`, tối ưu hiệu năng
- Ngày 5-8: Kiểm thử tích hợp, kiểm thử hệ thống
- Ngày 9-11: Hoàn thiện tài liệu
- Ngày 12-14: Fix lỗi cuối, tag phiên bản 1.0.0, push lên GitHub

**Các tính năng hoàn thành:**
- Lệnh `tr -info <file>`: Hiển thị thông tin chi tiết
- Kiểm thử tích hợp toàn bộ
- Tài liệu sử dụng hoàn chỉnh
- **Chuyển đổi metadata từ JSON sang SQLite3** (Issue #6): Thay thế parse JSON thủ công bằng SQLite3 embedded database, khắc phục bug parse offset, tăng hiệu năng truy vấn

**Đánh giá:** Dự án hoàn thành đúng hạn. Tất cả 10 User Story đã hoàn thành. Tổng story point: 30. Bug #6 (original_path parse error) đã được khắc phục bằng cách chuyển sang SQLite3.

---

## 7.4. Kết luận chương 7

Chương 7 đã trình bày chi tiết quá trình lập kế hoạch và thực hiện dự án theo phương pháp Scrum với 4 Sprint. Mỗi Sprint đã triển khai thành công các User Story từ Product Backlog, đáp ứng các tiêu chí chấp nhận đã đề ra. Dự án đã phát triển từ phiên bản V0 (chức năng cơ bản) đến phiên bản V3.0 (hoàn thiện, sẵn sàng phát hành).
