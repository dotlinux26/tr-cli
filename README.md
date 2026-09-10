# tr-cli

**Phát triển dự án phần mềm tiện ích hỗ trợ xóa tạm và khôi phục dữ liệu trên hệ điều hành Linux**

`tr` — Công cụ dòng lệnh thay thế `rm`, chuyển file vào thùng rác thay vì xóa vĩnh viễn. Hỗ trợ khôi phục, xem lịch sử, xóa vĩnh viễn khi cần.

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
- [Test cases](#test-cases)
- [License](#license)

---

## Tính năng

- **Xóa an toàn** — File vào thùng rác, không mất vĩnh viễn
- **Khôi phục** — Restore theo ID bất cứ lúc nào
- **Xóa vĩnh viễn** — `tr -rf` tương đương `rm -rf` khi cần
- **Giữ thư mục gốc** — `tr -rf folder/*` xóa con giữ folder
- **Xem lịch sử** — Liệt kê toàn bộ file đã xóa
- **Metadata đầy đủ** — Lưu lệnh gốc, thời gian, đường dẫn, dung lượng
- **Fail-safe** — Không tự ghi đè khi restore, luôn hỏi trước
- **Lightweight** — Chỉ cần SQLite3, build bằng CMake, chạy trên Linux

---

## So sánh rm vs tr

| rm (nguy hiểm) | tr (an toàn) | Ghi chú |
|-----------------|--------------|---------|
| `rm file.txt` | `tr file.txt` | Vào trash, có thể restore |
| `rm -i file.txt` | `tr file.txt` | tr đã safe sẵn |
| `rm -f file.txt` | `tr -f file.txt` | Bỏ confirm |
| `rm -r folder/` | `tr -r folder/` | Recursive vào trash |
| `rm -rf folder/` | `tr -rf folder/` | **Real delete** = xóa thẳng |
| `rm -rf folder/*` | `tr -rf folder/*` | Giữ folder, xóa con |
| _(không có)_ | `tr restore <ID>` | Khôi phục |
| _(không có)_ | `tr list` | Xem trash |
| _(không có)_ | `tr empty` | Dọn trash |
| _(không có)_ | `tr info <ID>` | Xem chi tiết |

---

## Cài đặt

### Yêu cầu

- Linux (POSIX)
- C++ compiler (GCC ≥ 9 hoặc Clang ≥ 10)
- CMake ≥ 3.14
- SQLite3
- nlohmann/json (header-only, included trong build)

### Build từ source

```bash
git clone https://github.com/dotlinux26/tr-cli.git
cd tr-cli
mkdir build && cd build
cmake ..
make
```

### Cài đặt

```bash
# Cài cho hệ thống (cần sudo)
sudo make install

# Hoặc cài cho user
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
```

---

## Sử dụng

### Xóa file (vào trash)

```bash
tr file.txt                    # chuyển file vào trash
tr file1.txt file2.txt         # nhiều file
tr -r folder/                  # chuyển cả folder vào trash
tr -f *.log                    # bỏ qua confirm
```

### Xóa vĩnh viễn (tương đương rm -rf)

```bash
tr -rf file.txt                # xóa thẳng, không qua trash
tr -rf folder/                 # xóa cả folder
tr -rf folder/*                # giữ folder, xóa hết con bên trong
```

### Xem danh sách trash

```bash
tr list                        # đầy đủ
tr -l                          # viết tắt
```

Output:

```
ID        Date                 Original Path              Type      Size
--------  -------------------  -------------------------  --------  --------
a8f21     2026-09-10 11:43:01  /home/user/file.txt        file      4.0 KB
c91de     2026-09-10 11:43:05  /home/user/project         directory 128.0 KB
```

### Khôi phục

```bash
tr restore a8f21               # khôi phục theo ID
tr -re a8f21                   # viết tắt
tr restore 3                   # theo thứ tự trong list
tr -ref a8f21                  # khôi phục + ghi đè nếu đã tồn tại
```

### Xóa vĩnh viễn toàn bộ trash

```bash
tr empty                       # đầy đủ
tr -e                          # viết tắt
```

### Xem thông tin chi tiết

```bash
tr info a8f21                  # đầy đủ
tr -i a8f21                    # viết tắt
```

Output:

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

## Tham chiếu lệnh

### Tất cả flags

| Flag | Nghĩa | Ví dụ |
|------|-------|-------|
| _(none)_ | Trash file(s) | `tr file.txt` |
| `-r` | Recursive | `tr -r folder/` |
| `-f` | Force (bỏ confirm) | `tr -f file.txt` |
| `-rf` | **Real delete** (xóa vĩnh viễn) | `tr -rf folder/` |
| `-rf *` | Xóa con giữ folder | `tr -rf folder/*` |
| `-l` | List | `tr -l` |
| `-re` | Restore | `tr -re <ID>` |
| `-ref` | Restore + overwrite | `tr -ref <ID>` |
| `-i` | Info | `tr -i <ID>` |
| `-e` | Empty trash | `tr -e` |

### Flow xử lý

**Trash:**
```
tr file.txt → Tạo ID → move vào ~/.local/share/trash/<ID>/data/ → ghi metadata.json
```

**Restore:**
```
tr restore <ID> → Đọc metadata → check conflict → move về original_path → xóa trash entry
```

**Real delete:**
```
tr -rf file.txt → rm -rf file.txt (không qua trash, không thể khôi phục)
```

---

## Cấu trúc thư mục trash

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
└── trash.db              # SQLite (backup metadata)
```

**ID format:** `{YYYYMMDD}-{HHmmss}-{5-char-hex}`

**Metadata example:**
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
├── CMakeLists.txt
├── LICENSE
├── README.md
├── SPEC.MD
├── src/
│   ├── main.cpp
│   ├── commands/
│   │   ├── trash.cpp / .h
│   │   ├── real_delete.cpp / .h
│   │   ├── list.cpp / .h
│   │   ├── restore.cpp / .h
│   │   ├── empty.cpp / .h
│   │   └── info.cpp / .h
│   ├── core/
│   │   ├── metadata.cpp / .h
│   │   ├── id_generator.cpp / .h
│   │   └── trash_store.cpp / .h
│   └── utils/
│       ├── fs_utils.cpp / .h
│       └── format.cpp / .h
├── tests/
│   ├── test_trash.sh
│   └── test_metadata.cpp
├── docs/
│   └── spec.md
└── TAILIEUTHAMKHAO/
    └── *.pdf
```

---

## Test cases

```bash
# Test 1: Trash file
echo "test" > /tmp/testfile.txt
tr /tmp/testfile.txt
tr list | grep testfile

# Test 2: Trash directory
mkdir -p /tmp/testdir/sub && touch /tmp/testdir/sub/file.txt
tr -r /tmp/testdir
tr list | grep testdir

# Test 3: Real delete
echo "delete me" > /tmp/delme.txt
tr -rf /tmp/delme.txt
ls /tmp/delme.txt 2>&1 | grep "No such"

# Test 4: Delete contents, keep folder
mkdir -p /tmp/keepdir && touch /tmp/keepdir/a.txt
tr -rf /tmp/keepdir/*
ls /tmp/keepdir

# Test 5: Restore
ID=$(tr list | head -2 | awk '{print $1}')
tr restore $ID

# Test 6: Restore conflict
touch /tmp/conflict.txt && tr /tmp/conflict.txt && touch /tmp/conflict.txt
tr -re <ID>

# Test 7: Empty trash
tr empty <<< "y"
```

---

## License

[MIT](LICENSE) — Copyright (c) 2026 Nguyễn Đức Cảnh
