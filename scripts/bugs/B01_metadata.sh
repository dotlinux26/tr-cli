#!/bin/bash
# B01: Khong tao duoc metadata.json khi trash file
# Anh: screenshots/B01_metadata.png
# Mo ta: File duoc di chuyen vao trash nhung metadata.json khong duoc tao

echo "=== B01: Khong tao metadata.json khi trash file ==="
echo ""

# Chuan bi
rm -rf ~/.local/share/trash/*
touch /tmp/testfile.txt

echo "1. Tao file test: touch /tmp/testfile.txt"
echo "2. Chay: tr /tmp/testfile.txt"
../build/tr /tmp/testfile.txt

echo ""
echo "3. Kiem tra trash:"
find ~/.local/share/trash/ -type f

echo ""
echo "4. Kiem tra metadata.json:"
if [ -f ~/.local/share/trash/*/metadata.json ]; then
    echo "metadata.json TON TAI:"
    cat ~/.local/share/trash/*/metadata.json
else
    echo "metadata.json KHONG TON TAI - LOI!"
fi

echo ""
echo "=== Ket thuc B01 ==="
