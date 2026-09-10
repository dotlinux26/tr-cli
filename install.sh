#!/usr/bin/env bash
# Script cai dat trash-cli cho he thong Linux
# Cai vao /usr/local/bin de goi "tr" tu bat cu dau
set -e

VERSION="1.0.0"
BUILD_DIR="build-release"

echo "=== Trash CLI Installer v${VERSION} ==="
echo ""

# 1. Build release
echo "[1/4] Build phien ban release..."
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j$(nproc)

# 2. Chay kiem thu
echo ""
echo "[2/4] Chay kiem thu..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

# 3. Cai dat vao he thong
echo ""
echo "[3/4] Cai dat vao /usr/local/bin..."
if [ "$(id -u)" -ne 0 ]; then
    echo "Can sudo de cai dat vao he thong."
    sudo cmake --install "$BUILD_DIR"
else
    cmake --install "$BUILD_DIR"
fi

# 4. Kiem tra
echo ""
echo "[4/4] Xac nhan cai dat..."
if command -v tr >/dev/null 2>&1; then
    echo "Cai dat thanh cong! Go 'tr --help' de xem huong dan."
    tr --version
else
    echo "Loi: khong tim thay lenh tr trong PATH."
    exit 1
fi