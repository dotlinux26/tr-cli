# Product Backlog

## 2.2.1. Phát triển yêu cầu dựa trên câu chuyện người dùng

Sử dụng định dạng tiêu chuẩn: "As a [user], I want [goal] so that [reason]"

Bảng 2.1. Danh sách các user story trong Product backlog

| TT | Theme | Epic | UID | User story |
|----|-------|------|-----|------------|
| 1 | Quản lý file | Xóa file | US-001 | As a người dùng Linux, I want xóa file bằng lệnh `tr` thay vì `rm` so that file được đưa vào thùng rác và có thể khôi phục nếu cần |
| 2 | Quản lý file | Xóa vĩnh viễn | US-002 | As a người dùng Linux, I want xóa file vĩnh viễn bằng lệnh `tr -rf` so that file bị loại bỏ hoàn toàn khỏi hệ thống |
| 3 | Quản lý file | Xóa nội dung giữ thư mục | US-003 | As a người dùng Linux, I want xóa nội dung bên trong thư mục bằng `tr -rf *` so that thư mục gốc vẫn được giữ lại |
| 4 | Quản lý file | Xóa đệ quy | US-004 | As a người dùng Linux, I want xóa đệ quy thư mục bằng `tr -r` so that toàn bộ nội dung được đưa vào thùng rác |
| 5 | Quản lý file | Khôi phục file | US-005 | As a người dùng Linux, I want khôi phục file từ thùng rác bằng `tr -re` so that file được trả về vị trí gốc |
| 6 | Quản lý file | Khôi phục ghi đè | US-006 | As a người dùng Linux, I want khôi phục file và ghi đè file đã tồn tại bằng `tr -ref` so that không cần xác nhận thủ công |
| 7 | Thao tác thùng rác | Liệt kê file | US-007 | As a người dùng Linux, I want xem danh sách file trong thùng rác bằng `tr -l` so that tôi biết những gì đã xóa |
| 8 | Thao tác thùng rác | Xem thông tin file | US-008 | As a người dùng Linux, I want xem thông tin chi tiết file trong thùng rác bằng `tr -i` so that tôi biết đường dẫn gốc và ngày xóa |
| 9 | Thao tác thùng rác | Xóa vĩnh viễn tất cả | US-009 | As a người dùng Linux, I want xóa toàn bộ nội dung thùng rác bằng `tr -e` so that tôi giải phóng dung lượng ổ đĩa |
| 10 | Thao tác thùng rác | Thông tin chi tiết | US-010 | As a người dùng Linux, I want xem thông tin chi tiết hơn bằng `tr -info` so that tôi biết kích thước, quyền truy cập của file |

## 2.2.2. Xác định tiêu chí chấp nhận cho các câu chuyện người dùng

Bảng 2.2. Danh sách user story cùng tiêu chí chấp nhận

| UID | As a... | I want to be able to... | So that... | Acceptance Criteria | Testing |
|-----|---------|------------------------|------------|--------------------|---------|
| US-001 | người dùng Linux | xóa file bằng lệnh `tr <file>` | file được đưa vào thùng rác và có thể khôi phục | File được di chuyển đến `~/.local/share/trash/YYYYMMDD-HHmmss-5hex/data/`; File `metadata.json` được tạo; File gốc không còn ở vị trí ban đầu | Chạy `tr testfile.txt` → kiểm tra file biến mất, kiểm tra file tồn tại trong trash |
| US-002 | người dùng Linux | xóa file vĩnh viễn bằng `tr -rf <file>` | file bị loại bỏ hoàn toàn khỏi hệ thống | File bị xóa vĩnh viễn; Không có bản sao trong trash; Trả về mã lỗi nếu file không tồn tại | Chạy `tr -rf testfile.txt` → kiểm tra file biến mất hoàn toàn |
| US-003 | người dùng Linux | xóa nội dung giữ thư mục bằng `tr -rf *` | thư mục gốc vẫn được giữ lại | Tất cả file và thư mục con bị xóa; Thư mục gốc vẫn tồn tại và rỗng | Tạo thư mục có nội dung → chạy `tr -rf *` → kiểm tra thư mục rỗng |
| US-004 | người dùng Linux | xóa đệ quy thư mục bằng `tr -r <dir>` | toàn bộ nội dung được đưa vào thùng rác | Tất cả file và thư mục con được di chuyển vào trash; Cấu trúc thư mục được bảo toàn; `metadata.json` ghi đường dẫn gốc | Tạo thư mục cấp 2 → chạy `tr -r testdir` → kiểm tra cấu trúc trong trash |
| US-005 | người dùng Linux | khôi phục file từ thùng rác bằng `tr -re <path>` | file được trả về vị trí gốc | File được di chuyển từ trash về vị trí gốc; File bị xóa khỏi trash; Hiển thị cảnh báo nếu file đã tồn tại | Trash file → chạy `tr -re <path>` → kiểm tra file về vị trí gốc |
| US-006 | người dùng Linux | khôi phục file ghi đè bằng `tr -ref <path>` | không cần xác nhận thủ công | File được ghi đè tại vị trí gốc; File bị xóa khỏi trash | Tạo file trùng tên → chạy `tr -ref <path>` → kiểm tra file được ghi đè |
| US-007 | người dùng Linux | xem danh sách file trong thùng rác bằng `tr -l` | tôi biết những gì đã xóa | Hiển thị tên file, đường dẫn gốc, ngày giờ xóa; Sắp xếp theo thời gian mới nhất; Hiển thị thông báo nếu trash trống | Chạy `tr -l` → kiểm tra danh sách hiển thị đúng |
| US-008 | người dùng Linux | xem thông tin file bằng `tr -i <file>` | tôi biết đường dẫn gốc và ngày xóa | Hiển thị tên file, kích thước, đường dẫn gốc, ngày xóa, loại file; Thông tin từ `metadata.json` | Trash file → chạy `tr -i <file>` → kiểm tra thông tin đúng |
| US-009 | người dùng Linux | xóa toàn bộ thùng rác bằng `tr -e` | tôi giải phóng dung lượng ổ đĩa | Tất cả file và thư mục con bị xóa; Thư mục trash gốc vẫn giữ lại; Hiển thị xác nhận trước khi xóa | Trash nhiều file → chạy `tr -e` → kiểm tra trash trống |
| US-010 | người dùng Linux | xem thông tin chi tiết bằng `tr -info <file>` | tôi biết kích thước, quyền truy cập | Hiển thị tên file, kích thước (byte/KB/MB), đường dẫn gốc, ngày xóa, loại file, quyền truy cập | Chạy `tr -info <file>` → kiểm tra đầy đủ thông tin |

## 2.2.3. Ước tính độ phức tạp câu chuyện người dùng

Sử dụng kỹ thuật Planning Poker để ước tính story point và Estimation Effort.

Bảng 2.3. Danh sách user story với các giá trị ước tính Story point và Estimation Effort

| UID | User story | Story point | Estimation Effort |
|-----|------------|-------------|-------------------|
| US-001 | Xóa file vào thùng rác (`tr`) | 3 | 4 giờ |
| US-002 | Xóa file vĩnh viễn (`-rf`) | 2 | 2 giờ |
| US-003 | Xóa nội dung giữ thư mục (`-rf *`) | 3 | 4 giờ |
| US-004 | Xóa đệ quy (`-r`) | 5 | 8 giờ |
| US-005 | Khôi phục file (`-re`) | 5 | 8 giờ |
| US-006 | Khôi phục ghi đè (`-ref`) | 3 | 4 giờ |
| US-007 | Liệt kê file trong trash (`-l`) | 2 | 3 giờ |
| US-008 | Xem thông tin file (`-i`) | 2 | 3 giờ |
| US-009 | Xóa vĩnh viễn tất cả file (`-e`) | 2 | 2 giờ |
| US-010 | Hiển thị thông tin chi tiết (`-info`) | 3 | 4 giờ |

**Tổng story point:** 30
**Tổng estimation effort:** 42 giờ

## 2.2.4. Tạo bản đồ câu chuyện và tinh chỉnh các câu chuyện người dùng

Bảng 2.4. Danh sách user story sau tinh chỉnh (sắp xếp theo ưu tiên)

| UID | User story | Story point | Estimation Effort | Thứ tự ưu tiên |
|-----|------------|-------------|-------------------|----------------|
| US-001 | Xóa file vào thùng rác (`tr`) | 3 | 4 giờ | 1 |
| US-007 | Liệt kê file trong trash (`-l`) | 2 | 3 giờ | 2 |
| US-009 | Xóa vĩnh viễn tất cả file (`-e`) | 2 | 2 giờ | 3 |
| US-004 | Xóa đệ quy (`-r`) | 5 | 8 giờ | 4 |
| US-005 | Khôi phục file (`-re`) | 5 | 8 giờ | 5 |
| US-008 | Xem thông tin file (`-i`) | 2 | 3 giờ | 6 |
| US-006 | Khôi phục ghi đè (`-ref`) | 3 | 4 giờ | 7 |
| US-002 | Xóa file vĩnh viễn (`-rf`) | 2 | 2 giờ | 8 |
| US-003 | Xóa nội dung giữ thư mục (`-rf *`) | 3 | 4 giờ | 9 |
| US-010 | Hiển thị thông tin chi tiết (`-info`) | 3 | 4 giờ | 10 |
