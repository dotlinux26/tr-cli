# Trash CLI (tr)

**Phát triển dự án phần mềm tiện ích hỗ trợ xóa tạm và khôi phục dữ liệu trên hệ điều hành Linux**

`tr` là công cụ dòng lệnh thay thế `rm`, chuyển file vào thùng rác thay vì xóa vĩnh viễn. Hỗ trợ khôi phục, liệt kê, xem thông tin và xóa vĩnh viễn khi cần.

```
  ████████╗███████╗██████╗ ███╗   ███╗
  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║
     ██║   █████╗  ██████╔╝██╔████╔██║
     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║
     ██║   ███████╗██║  ██║██║ ╚═╝ ██║
     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝
```

**Phiên bản:** 1.0.0 | **Tác giả:** Nguyễn Đức Cảnh | **Giấy phép:** MIT

---

## Mục lục

- [Tính năng](#tính-năng)
- [So sánh rm vs tr](#so-sánh-rm-vs-tr)
- [Cài đặt](#cài-đặt)
- [Sử dụng](#sử-dụng)
- [Tham chiếu lệnh](#tham-chiếu-lệnh)
- [Cấu trúc thư mục trash](#cấu-trúc-thư-mục-trash)
- [Công nghệ](#công-nghệ)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Kiểm thử](#kiểm-thử)
- [License](#license)

---

## Tính năng

- **Xóa an toàn** — File vào thùng rác, không mất vĩnh viễn
- **Khôi phục** — Trả file về vị trí gốc theo đường dẫn đã lưu
- **Xóa vĩnh viễn** — `tr -rf` tương đương `rm -rf` khi cần
- **Giữ thư mục gốc** — `tr -rf *` xóa nội dung, giữ thư mục
- **Liệt kê** — Xem toàn bộ file đã xóa cùng ngày giờ
- **Thông tin chi tiết** — Kích thước, quyền truy cập, đường dẫn gốc
- **Metadata đầy đủ** — Lưu tên, đường dẫn, thời gian, dung lượng
- **Fail-safe** — Không tự ghi đè khi khôi phục, luôn cảnh báo

---

## So sánh rm vs tr

| rm (nguy hiểm) | tr (an toàn) | Ghi chú |
|-----------------|--------------|---------|
| `rm file.txt` | `tr file.txt` | Vào trash, có thể khôi phục |
| `rm -r folder/` | `tr -r folder/` | Đệ quy vào trash |
| `rm -rf folder/` | `tr -rf folder/` | **Xóa thẳng** = xóa vĩnh viễn |
| `rm -rf folder/*` | `tr -rf *` | Giữ thư mục, xóa nội dung |
| _(không có)_ | `tr -re <path>` | Khôi phục |
| _(không có)_ | `tr -l` | Xem danh sách trash |
| _(không có)_ | `tr -e` | Dọn trash |
| _(không có)_ | `tr -i <file>` | Xem thông tin |

---

## Cài đặt

### Yêu cầu

- Linux (POSIX)
- C++ compiler (GCC ≥ 9 hoặc Clang ≥ 10)
- CMake ≥ 3.14 (khi build từ source)

### Cách 1: Gói cài đặt Debian (.deb)

```bash
# Debian / Ubuntu
sudo dpkg -i trashcli_1.0.0_amd64.deb
```

### Cách 2: Gói nén (tar.gz / zip)

```bash
# Trích xuất
tar -xzf trashcli-v1.0.0-linux-x86_64.tar.gz

# Cài cho hệ thống
sudo mv tr /usr/local/bin/tr

# Hoặc cài cho user
mkdir -p ~/.local/bin
mv tr ~/.local/bin/tr
export PATH="$HOME/.local/bin:$PATH"
```

### Cách 3: Script cài đặt tự động

```bash
chmod +x install.sh
./install.sh
```

Script tự động build, chạy kiểm thử và cài đặt vào hệ thống.

### Cách 4: Build từ source

```bash
git clone https://github.com/dotlinux26/tr-cli.git
cd tr-cli
mkdir build && cd build
cmake ..
make
```

### Cài đặt sau build

```bash
# Cài cho hệ thống (cần sudo)
sudo cmake --install .

# Hoặc chỉ định vị trí cài
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make && make install
```

Đảm bảo `~/.local/bin` nằm trong `$PATH`:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

### Kiểm tra

```bash
which tr
tr --version
tr --help
```

---

## Sử dụng

### Xóa file (vào trash)

```bash
tr file.txt                    # chuyển file vào trash
tr folder/                     # chuyển folder vào trash
tr -r folder/                  # chuyển folder đệ quy vào trash
```

### Xóa vĩnh viễn (tương đương rm -rf)

```bash
tr -rf file.txt                # xóa thẳng, không qua trash
tr -rf folder/                 # xóa cả folder
tr -rf *                       # giữ thư mục hiện tại, xóa toàn bộ nội dung
```

### Xem danh sách trash

```bash
tr -l
```

Output:

```

  ████████╗███████╗██████╗ ███╗   ███╗
  ...

  Danh sách thùng rác:
  ─────────────────────────────────────────────────
  testfile.txt
    Gốc:    /tmp/testfile.txt
    Xóa:    2026-09-10 20:51:50

  mydir
    Gốc:    /tmp/mydir
    Xóa:    2026-09-10 20:55:12
```

### Khôi phục

```bash
tr -re ~/.local/share/trash/20260910-205150-2b42b/data/testfile.txt
```

Hoặc khôi phục và ghi đè nếu file đã tồn tại:

```bash
tr -ref ~/.local/share/trash/20260910-205150-2b42b/data/testfile.txt
```

### Xóa vĩnh viễn toàn bộ trash

```bash
tr -e
```

Chương trình sẽ hỏi xác nhận trước khi xóa.

### Xem thông tin

```bash
tr -i file.txt                 # thông tin cơ bản
tr -info file.txt              # thông tin chi tiết (quyền truy cập, kích thước chính xác)
```

### Trợ giúp

```bash
tr -h
tr --help
tr -v
tr --version
```

---

## Tham chiếu lệnh

### Tất cả flags

| Flag | Nghĩa | Ví dụ |
|------|-------|-------|
| _(không)_ | Xóa file vào trash | `tr file.txt` |
| `-r` | Xóa đệ quy thư mục | `tr -r folder/` |
| `-rf` | **Xóa vĩnh viễn** | `tr -rf folder/` |
| `-rf *` | Xóa nội dung, giữ thư mục | `tr -rf *` |
| `-re` | Khôi phục | `tr -re <path>` |
| `-ref` | Khôi phục ghi đè | `tr -ref <path>` |
| `-l` | Liệt kê | `tr -l` |
| `-i` | Thông tin | `tr -i <file>` |
| `-info` | Thông tin chi tiết | `tr -info <file>` |
| `-e` | Xóa vĩnh viễn toàn bộ trash | `tr -e` |
| `-h`, `--help` | Hiển thị trợ giúp | `tr -h` |
| `-v`, `--version` | Hiển thị phiên bản | `tr -v` |

### Flow xử lý

**Trash:**
```
tr file.txt → Tạo thư mục YYYYMMDD-HHmmss-5hex → di chuyển file vào data/ → ghi metadata.json
```

**Restore:**
```
tr -re <path> → Đọc metadata.json → kiểm tra xung đột → di chuyển về vị trí gốc → xóa thư mục trash
```

**Real delete:**
```
tr -rf file.txt → xóa thẳng khỏi hệ thống (không qua trash, không thể khôi phục)
```

---

## Cấu trúc thư mục trash

```
~/.local/share/trash/
├── 20260910-205150-2b42b/
│   ├── data/
│   │   └── file.txt
│   └── metadata.json
├── 20260910-205230-abc12/
│   ├── data/
│   │   └── project/
│   │       ├── a.txt
│   │       └── b.txt
│   └── metadata.json
```

**Định dạng thư mục:** `{YYYYMMDD}-{HHmmss}-{5-char-hex}`

**Metadata example:**
```json
{
  "name": "file.txt",
  "original_path": "/home/user/project/file.txt",
  "deleted_at": "2026-09-10 20:51:50",
  "size": 4096
}
```

---

## Công nghệ

| Thành phần | Lựa chọn |
|------------|----------|
| Ngôn ngữ | C++17 |
| Build system | CMake ≥ 3.14 |
| Database | SQLite3 (embedded) |
| JSON | nlohmann/json |
| Nền tảng | Linux (POSIX) |
| Giấy phép | MIT |

---

## Cấu trúc dự án

```
tr-cli/
├── CMakeLists.txt          # Hệ thống build + cài đặt
├── install.sh              # Script cài đặt tự động
├── LICENSE                 # Giấy phép MIT
├── README.md               # Tài liệu này
├── SPEC.MD                 # Đặc tả chức năng
├── include/
│   └── trashcli.h          # Header chính
├── src/
│   ├── main.cpp            # Điểm vào + CLI parser
│   └── trash.cpp           # Xử lý chính
├── test/
│   └── test_trash.cpp      # Đơn vị kiểm thử
├── docs/
│   ├── 01-architecture.md  # Kiến trúc hệ thống
│   ├── 02-database.md      # Hệ thống SQL
│   ├── 03-modules.md       # Thiết kế module
│   ├── 04-opensource.md    # Công nghệ nguồn mở
│   ├── backlog.md          # Product backlog
│   ├── sprint-log.md       # Nhật ký sprint
│   └── process.md          # Quy trình phát triển
├── scripts/
│   └── bugs/               # Script mô phỏng lỗi
│       ├── B01_metadata.sh
│       ├── B02_crash.sh
│       ├── B03_permission.sh
│       ├── B04_duplicate.sh
│       └── B05_relative_path.sh
├── dist/                   # Gói phát hành
│   ├── trashcli_1.0.0_amd64.deb
│   ├── trashcli-v1.0.0-linux-x86_64.tar.gz
│   └── trashcli-v1.0.0-linux-x86_64.zip
└── thirdparty/
    ├── json/               # nlohmann/json header
    └── sqlite/             # SQLite3 header
```

---

## Kiểm thử

### Đơn vị kiểm thử

```bash
# Build và chạy
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Output:

```
1/1 Test #1: test_trash .......................   Passed    0.01 sec
100% tests passed, 0 tests failed out of 1
```

### Kiểm thử thủ công

```bash
# Test 1: Trash file
echo "test" > /tmp/testfile.txt
tr /tmp/testfile.txt
tr -l | grep testfile

# Test 2: Trash directory
mkdir -p /tmp/testdir/sub && touch /tmp/testdir/sub/file.txt
tr -r /tmp/testdir
tr -l | grep testdir

# Test 3: Real delete
echo "delete me" > /tmp/delme.txt
tr -rf /tmp/delme.txt
ls /tmp/delme.txt 2>&1 | grep "No such"

# Test 4: Delete contents, keep folder
mkdir -p /tmp/keepdir && touch /tmp/keepdir/a.txt
cd /tmp/keepdir && tr -rf * && ls /tmp/keepdir

# Test 5: Restore
tr -re ~/.local/share/trash/*/data/testfile.txt
ls /tmp/testfile.txt

# Test 6: Restore conflict
touch /tmp/conflict.txt && tr /tmp/conflict.txt && touch /tmp/conflict.txt
tr -re ~/.local/share/trash/*/data/conflict.txt

# Test 7: Empty trash
echo "y" | tr -e
```

---

## License

[MIT](LICENSE) — Copyright (c) 2026 Nguyễn Đức Cảnh