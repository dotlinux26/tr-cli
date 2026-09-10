# 05. Quy trình phát triển dự án

## 5.1. Mô hình phát triển: Agile - Scrum

Dự án sử dụng phương pháp Agile với khuôn khổ Scrum để quản lý quá trình phát triển phần mềm. Scrum là một khuôn khổ linh hoạt, dựa trên các Sprint lặp đi lặp lại, mỗi Sprint tạo ra một phần tăng trưởng có thể phát hành được (potentially releasable increment).

### Lý do chọn Scrum

- Dự án có quy mô nhỏ đến trung bình, cần linh hoạt trong việc thay đổi yêu cầu
- Mỗi Sprint tạo ra một sản phẩm có thể demo được
- Tính tự quản (self-organizing) của nhóm phát triển giúp tối ưu hiệu suất
- Phù hợp với thời gian thực hiện của sinh viên (1 học kỳ)

## 5.2. Các vai trò trong dự án

| Vai trò | Thành viên | Trách nhiệm |
|---------|-----------|-------------|
| Product Owner | Nguyễn Đức Cảnh | Quản lý Product Backlog, ưu tiên hóa các hạng mục, đảm bảo giá trị sản phẩm |
| Scrum Master | Nguyễn Đức Cảnh | Đảm bảo nhóm tuân thủ Scrum, loại bỏ rào cản, huấn luyện nhóm |
| Developer | Nguyễn Đức Cảnh | Triển khai mã nguồn, kiểm thử, đảm bảo chất lượng sản phẩm |
| Technical Writer | Lò Thanh Tùng | Viết tài liệu quy trình, báo cáo, hướng dẫn sử dụng |
| Technical Writer | Nguyễn Khắc Nam Khánh | Viết tài liệu quy trình, báo cáo, hướng dẫn sử dụng |

## 5.3. Các sự kiện Scrum

### Sprint Planning (Lập kế hoạch Sprint)

- **Thời gian:** Đầu mỗi Sprint
- **Mục đích:** Xác định mục tiêu Sprint (Sprint Goal), lựa chọn các hạng mục từ Product Backlog, chia nhỏ thành các task
- **Tham gia:** Toàn bộ Scrum Team

### Daily Stand-Up (Họp hàng ngày)

- **Thời gian:** Mỗi ngày, kéo dài khoảng 15 phút
- **Mục đích:** Cập nhật tiến độ, lập kế hoạch ngắn hạn, nhận diện vấn đề
- **Nội dung:** Mỗi thành viên trả lời 3 câu hỏi:
  1. Hôm qua đã làm gì?
  2. Hôm nay sẽ làm gì?
  3. Có rào cản nào không?

### Sprint Review (Đánh giá Sprint)

- **Thời gian:** Cuối mỗi Sprint
- **Mục đích:** Demo sản phẩm cho các bên liên quan, nhận phản hồi
- **Sản phẩm:** Phần tăng trưởng (Increment) có thể phát hành

### Sprint Retrospective (Tổng kết Sprint)

- **Thời gian:** Sau Sprint Review
- **Mục đích:** Đánh giá quá trình làm việc, xác định cải tiến cho Sprint tiếp theo
- **Nội dung:** Điểm mạnh, điểm yếu, hành động cải tiến

## 5.4. Quy trình quản lý yêu cầu

### Product Backlog

Product Backlog là danh sách tất cả các công việc cần thực hiện để hoàn thành dự án. Đặc điểm của Product Backlog:

- **Detailed appropriately (Chi tiết hợp lý):** Các hạng mục quan trọng được chi tiết hóa, hạng mục thấp ưu tiên ít chi tiết hơn
- **Estimated (Ước lượng):** Mỗi hạng mục có story point và estimation effort
- **Emergent (Tiến hóa):** Liên tục được cập nhật theo thời gian
- **Prioritized (Ưu tiên hóa):** Sắp xếp theo giá trị từ cao đến thấp

### User Story

Mỗi yêu cầu được viết dưới dạng User Story với định dạng:

```
As a [tác nhân], I want [mục tiêu] so that [lý do]
```

Mỗi User Story có:
- **UID:** Mã định dạng duy nhất
- **Acceptance Criteria:** Tiêu chí chấp nhận
- **Story Point:** Độ phức tạp ước tính (Planning Poker)
- **Estimation Effort:** Thời gian ước tính

### Sprint Backlog

Sprint Backlog là tập hợp con của Product Backlog, chứa các hạng mục được chọn cho Sprint hiện tại. Mỗi Sprint Backlog gồm:

- UID, User Story, Story Point, Estimation Effort
- Status, Acceptance Criteria, Testing
- Task owner, Task, Subtask

## 5.5. Các công cụ quản lý dự án

| Công cụ | Mục đích |
|---------|----------|
| GitHub | Quản lý mã nguồn, Issues, Projects (Sprint Board) |
| GitHub Issues | Theo dõi lỗi và yêu cầu tính năng |
| GitHub Projects | Bảng Sprint (To Do, In Progress, Done) |
| Git | Hệ thống kiểm soát phiên bản |
| CMake | Hệ thống build |
| Google Test | Framework kiểm thử đơn vị |

## 5.6. Kiểm thử trong Scrum

### Kiểm thử đơn vị (Unit Testing)

- Kiểm tra từng thành phần nhỏ nhất của mã nguồn
- Sử dụng Google Test framework
- Tự động hóa kiểm thử

### Kiểm thử tích hợp (Integration Testing)

- Đảm bảo các module hoạt động tốt khi kết hợp
- Kiểm tra dữ liệu truyền giữa các thành phần

### Kiểm thử hệ thống (System Testing)

- Kiểm tra toàn bộ hệ thống trong môi trường thực tế
- Kiểm tra chức năng và phi chức năng

### Kiểm thử chấp nhận (Acceptance Testing)

- Người dùng cuối kiểm tra sản phẩm
- Đảm bảo đáp ứng yêu cầu và kỳ vọng
