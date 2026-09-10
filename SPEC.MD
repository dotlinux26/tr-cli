# trash-cli (`tr`) — Specification v1.0

## 1. Tổng quan

`tr` là công cụ dòng lệnh thay thế `rm` trên Linux. Thay vì xóa vĩnh viễn,
`tr` chuyển file/thư mục vào thùng rác, cho phép khôi phục bất cứ lúc nào.

```bash
tr file.txt              # chuyển vào trash
tr -r folder/            # recursive → trash
tr -rf folder/           # REAL DELETE (rm -rf equivalent)
tr -rf folder/*          # xóa con giữ thư mục gốc
tr list                  # xem trash
tr restore <ID>          # khôi phục
tr empty                 # xóa vĩnh viễn trash
tr info <ID>             # xem metadata
```

---

## 2. Cài đặt

```bash
# Build từ source
mkdir build && cd build
cmake ..
make
sudo make install

# Hoặc cài cho user
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make && make install
```

Binary nằm ở `/usr/local/bin/tr` hoặc `~/.local/bin/tr`.
Đảm bảo thư mục nằm trong `$PATH`.

```bash
which tr          # kiểm tra
command -v tr     # chuẩn hơn
```

---

## 3. Flags tổng hợp

| Flag | Nghĩa | Ví dụ | Ghi chú |
|------|-------|-------|---------|
| _(none)_ | Trash file(s) | `tr file.txt` | Mặc định |
| `-r` | Recursive | `tr -r folder/` | Trash cả thư mục con |
| `-f` | Force | `tr -f file.txt` | Bỏ qua xác nhận |
| `-rf` | **Real delete** | `tr -rf folder/` | Xóa vĩnh viễn = `rm -rf` |
| `-rf *` | Delete contents | `tr -rf folder/*` | Giữ folder, xóa con |
| `-l` | List | `tr -l` | Alias cho `tr list` |
| `-re` | Restore | `tr -re <ID>` | Khôi phục (không overwrite) |
| `-ref` | Restore + force | `tr -ref <ID>` | Khôi phục + overwrite |
| `-i` | Info | `tr -i <ID>` | Alias cho `tr info` |
| `-e` | Empty | `tr -e` | Alias cho `tr empty` |

> **Lưu ý quan trọng:**
> - `-rf` = **REAL DELETE** (bỏ qua trash, xóa thẳng). Đây là behavior khác biệt hoàn toàn so với `tr` mặc định.
> - `-rf folder/*` = xóa tất cả nội dung bên trong folder nhưng **giữ lại folder gốc** (giống `rm -rf folder/*`).
> - `-re` / `-ref`: `e` = restore, `f` = force overwrite.

---

## 4. Cấu trúc thư mục trash

```
~/.local/share/trash/
├── 20260910-114301-a8f21/
│   ├── data/
│   │   └── file.txt
│   └── metadata.json
├── 20260910-114305-c91de/
│   ├── data/
│   │   └── project/
│   │       ├── a.txt
│   │       └── b.txt
│   └── metadata.json
└── trash.db              # SQLite (tùy chọn, metadata.json là chính)
```

Mỗi lần `tr` xử lý 1 target → tạo 1 thư mục con:
`{YYYYMMDD}-{HHmmss}-{5-char-hex}/`

---

## 5. Metadata format

`metadata.json` trong mỗi thư mục trash:

```json
{
  "id": "20260910-114301-a8f21",
  "command": "tr file.txt",
  "time": "2026-09-10T11:43:01+07:00",
  "original_path": "/home/user/project/file.txt",
  "type": "file",
  "size": 4096
}
```

| Trường | Kiểu | Mô tả |
|--------|------|-------|
| `id` | string | Định danh duy nhất |
| `command` | string | Lệnh gốc đã thực thi |
| `time` | string | ISO 8601 + timezone |
| `original_path` | string | Đường dẫn gốc |
| `type` | string | `"file"` hoặc `"directory"` |
| `size` | int | Kích thước (bytes) |

---

## 6. Chi tiết từng lệnh

### 6.1 `tr <target>` — Chuyển vào trash

```bash
tr file.txt
tr -r folder/
tr file1.txt file2.txt folder/      # nhiều target
tr -f *.log                         # force, bỏ confirm
```

**Flow:**

```
target tồn tại?
  ├── Không → Lỗi: "No such file or directory"
  └── Có
        ├── Directory + không có -r → Lỗi: "Is a directory (use -r)"
        └── OK → Tạo thư mục trash ID mới
              → move target vào data/
              → ghi metadata.json
              → In: "Trashed: /path/to/file → ID: a8f21"
```

### 6.2 `tr -rf <target>` — Xóa vĩnh viễn (Real Delete)

```bash
tr -rf file.txt           # xóa thẳng, không qua trash
tr -rf folder/            # xóa cả folder
tr -rf folder/*           # giữ folder, xóa con
```

**Flow:**

```
target tồn tại?
  ├── Không → Lỗi
  └── Có
        ├── folder/* → xóa nội dung bên trong, giữ folder gốc
        └── khác → rm -rf target
        → In: "Permanently deleted: /path/to/target"
```

> **LƯU Ý:** Lệnh này **KHÔNG** tạo metadata. File bị xóa vĩnh viễn, không thể khôi phục.
> Tương đương chính xác `rm -rf`.

### 6.3 `tr list` — Xem danh sách trash

```bash
tr list          # đầy đủ
tr -l            # viết tắt
```

**Output:**

```
ID        Date                 Original Path              Type      Size
--------  -------------------  -------------------------  --------  --------
a8f21     2026-09-10 11:43:01  /home/user/file.txt        file      4.0 KB
c91de     2026-09-10 11:43:05  /home/user/project         directory 128.0 KB
b3aa7     2026-09-10 12:00:00  /home/user/notes.txt       file      1.2 KB
```

**Flow:** Đọc tất cả `metadata.json` → sắp xếp giảm dần theo thời gian → hiển thị.

### 6.4 `tr restore <ID>` — Khôi phục

```bash
tr restore a8f21           # không overwrite
tr -re a8f21               # alias
tr restore 3               # theo thứ tự trong list
tr -ref a8f21              # restore + overwrite
tr restore a8f21 --force   # tương đương -ref
```

**Flow:**

```
Tìm thư mục trash theo ID
  ├── Không tìm thấy → Lỗi: "No trash entry found for ID: xxx"
  └── Tìm thấy
        → Đọc metadata.json → lấy original_path
        → original_path đã tồn tại?
              ├── Chưa có → mv data/* original_path/
              │            → xóa thư mục trash
              │            → In: "Restored: file.txt → /home/user/file.txt"
              │
              ├── Đã có + (--force hoặc -ref) → ghi đè
              │   → mv data/* original_path/
              │   → xóa thư mục trash
              │   → In: "Restored (overwritten): file.txt → /home/user/file.txt"
              │
              └── Đã có + KHÔNG force → CẢNH BÁO & HỦY:
                  [!] Restore target already exists:
                      /home/user/project/file.txt

                  Restore cancelled. No files were overwritten.
                  Use -ref or --force to overwrite.
```

### 6.5 `tr empty` — Xóa vĩnh viễn toàn bộ trash

```bash
tr empty         # đầy đủ
tr -e            # viết tắt
```

**Flow:**

```
Trash trống?
  ├── Có → In: "Trash is empty."
  └── Không
        → Liệt kê: "Found 5 items in trash."
        → Hỏi: "Delete all 5 items permanently? [y/N] "
        ├── y/Y → rm -rf ~/.local/share/trash/*
        │        → In: "Trash emptied."
        └── khác → "Cancelled."
```

### 6.6 `tr info <ID>` — Xem metadata

```bash
tr info a8f21       # đầy đủ
tr -i a8f21         # viết tắt
```

**Output:**

```
ID:          a8f21
Command:     tr file.txt
Time:        2026-09-10 11:43:01 (+07:00)
Original:    /home/user/project/file.txt
Type:        file
Size:        4096 bytes (4.0 KB)
Trash path:  ~/.local/share/trash/20260910-114301-a8f21/data/file.txt
```

---

## 7. Bảng mapping lệnh cũ → lệnh mới

| rm (nguy hiểm) | tr (an toàn) | Ghi chú |
|-----------------|--------------|---------|
| `rm file.txt` | `tr file.txt` | Vào trash, có thể restore |
| `rm -i file.txt` | `tr file.txt` | tr đã safe sẵn |
| `rm -f file.txt` | `tr -f file.txt` | Bỏ confirm |
| `rm -r folder/` | `tr -r folder/` | Recursive vào trash |
| `rm -rf folder/` | `tr -rf folder/` | **Real delete** = rm -rf |
| `rm -rf folder/*` | `tr -rf folder/*` | Giữ folder, xóa con |
| _(không có)_ | `tr restore <ID>` | Khôi phục (rm không làm được) |
| _(không có)_ | `tr list` | Xem trash |
| _(không có)_ | `tr empty` | Dọn trash |
| _(không có)_ | `tr info <ID>` | Xem chi tiết |

---

## 8. Công nghệ

| Thành phần | Lựa chọn | Lý do |
|------------|----------|-------|
| Ngôn ngữ | C++17 | Performance, filesystem API, structured bindings |
| Build system | CMake ≥ 3.14 | Cross-platform, find_package |
| Database | SQLite3 (embedded) | Metadata backup, query nhanh |
| JSON | nlohmann/json | metadata.json (header-only) |
| Nền tảng | Linux (POSIX) | rename(2), filesystem |
| VCS | Git | Theo yêu cầu học phần |
| Giấy phép | MIT | OSI approved (Bài 1 trang 13-15) |

**Dependencies:** Chỉ SQLite3 + nlohmann/json (header-only).
Cả hai đều OSS, phù hợp tiêu chí OSI.

---

## 9. Cấu trúc thư mục dự án

```
trash-cli/
├── CMakeLists.txt
├── LICENSE                   # MIT
├── README.md
├── CHANGELOG.md
├── src/
│   ├── main.cpp              # Entry point, arg parsing
│   ├── commands/
│   │   ├── trash.cpp         # tr <target> — move vào trash
│   │   ├── trash.h
│   │   ├── real_delete.cpp   # tr -rf — xóa vĩnh viễn
│   │   ├── real_delete.h
│   │   ├── list.cpp          # tr list
│   │   ├── list.h
│   │   ├── restore.cpp       # tr restore
│   │   ├── restore.h
│   │   ├── empty.cpp         # tr empty
│   │   ├── empty.h
│   │   ├── info.cpp          # tr info
│   │   └── info.h
│   ├── core/
│   │   ├── metadata.cpp      # Đọc/ghi metadata.json
│   │   ├── metadata.h
│   │   ├── id_generator.cpp  # Tạo ID {timestamp}-{hex}
│   │   ├── id_generator.h
│   │   ├── trash_store.cpp   # Quản lý thư mục trash
│   │   └── trash_store.h
│   └── utils/
│       ├── fs_utils.cpp      # Filesystem helpers
│       ├── fs_utils.h
│       ├── format.cpp        # Format time, size
│       └── format.h
├── tests/
│   ├── test_trash.sh         # Shell-based integration tests
│   ├── test_metadata.cpp     # Unit test metadata
│   └── CMakeLists.txt
└── docs/
    └── spec.md               # Tài liệu này
```

---

## 10. Yêu cầu kỹ thuật

### 10.1 ID & Thời gian

- ID format: `{YYYYMMDD}-{HHmmss}-{5hex}` (vd: `20260910-114301-a8f21`)
- Timezone: local timezone của hệ thống
- Nếu trùng ID (rất hiếm) → regenerate hex

### 10.2 Filesystem

- Symlink: giữ nguyên khi move (không dereference)
- Permissions: giữ nguyên quyền file gốc
- Large files: dùng `rename(2)` trước, fallback `copy + delete` nếu cross-filesystem
- Hardlink: giữ nguyên

### 10.3 Error handling

- `rename()` fail → rollback, không tạo metadata rác
- Permission denied → in lỗi rõ ràng, tiếp tục target khác
- Disk full khi copy → xóa partial copy, rollback
- Trash dir không tồn tại → tự tạo

### 10.4 Edge cases

- File không tồn tại → `tr: cannot trash 'x': No such file or directory`
- Target là symlink → move symlink, không dereference
- Target là `.` hoặc `..` → lỗi
- `tr -rf` trên file không tồn tại → im lặng (giống `rm -f`)
- Trash đầy → cảnh báo dung lượng

### 10.5 Alias flag

Để tránh conflict với `tr` của Unix (translate characters):

- Nếu binary tên `tr` → cần alias trong shell hoặc đổi tên binary
- Giải pháp: ưu tiên tên `trash` hoặc `rmx` nếu担心 conflict
- Hoặc dùng `command -v tr` để phân biệt

> **Khuyến nghị:** Đặt binary tên `trash` để tránh conflict hoàn toàn.

---

## 11. Deliverables

| # | Nội dung | Ghi chú |
|---|----------|---------|
| 1 | Source code hoàn chỉnh | GitHub/GitLab, có README |
| 2 | README.md | Hướng dẫn cài đặt, sử dụng, demo |
| 3 | LICENSE | MIT |
| 4 | CHANGELOG.md | Version history |
| 5 | docs/spec.md | Tài liệu spec này |
| 6 | Tests | Ít nhất 7 test cases |
| 7 | Báo cáo môn học | Đối chiếu yêu cầu ↔ sản phẩm |

---

## 12. Test cases

```bash
# Test 1: Trash file
echo "test" > /tmp/testfile.txt
tr /tmp/testfile.txt
tr list | grep testfile     # phải có

# Test 2: Trash directory
mkdir -p /tmp/testdir/sub
touch /tmp/testdir/sub/file.txt
tr -r /tmp/testdir
tr list | grep testdir      # phải có

# Test 3: Real delete
echo "delete me" > /tmp/delme.txt
tr -rf /tmp/delme.txt
ls /tmp/delme.txt 2>&1 | grep "No such"  # phải fail

# Test 4: Real delete folder/* (giữ folder)
mkdir -p /tmp/keepdir && touch /tmp/keepdir/a.txt
tr -rf /tmp/keepdir/*
ls /tmp/keepdir             # phải tồn tại
ls /tmp/keepdir/a.txt 2>&1 | grep "No such"  # con phải mất

# Test 5: Restore
ID=$(tr list | head -2 | awk '{print $1}')
tr restore $ID
tr list | grep $ID          # phải không còn

# Test 6: Restore conflict
touch /tmp/conflict.txt
tr /tmp/conflict.txt
touch /tmp/conflict.txt     # tạo lại
tr -re <ID>                 # phải cảnh báo conflict

# Test 7: Empty trash
tr empty <<< "y"
tr list                     # phải trống
```

---

## 13. Tài liệu tham khảo (trích dẫn trong báo cáo)

| Nguồn | Nội dung | Dùng ở đâu |
|-------|----------|------------|
| Bài 1, trang 6-10 | Tiêu chí OSI đánh giá OSS | Section 8: Công nghệ |
| Bài 1, trang 13-15 | Giấy phép MIT | Section 8: Giấy phép |
| Bài 2, trang 9 | Đặc điểm MIT | Section 8 |
| Bài 3, trang 1 | Mô hình phát triển PMNM | Quy trình dev |
| Bài 4, trang 1 | Tham gia cộng đồng OSS | GitHub/GitLab |
| Bài 5, trang 4-7 | Tiêu chí PoF | Đánh giá dự án |
