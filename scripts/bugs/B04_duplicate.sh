#!/bin/bash
# B04: Ten file trung nhau khi trash nhieu file cung ten
# Anh: screenshots/B04_duplicate.png
# Mo ta: Khi trash nhieu file cung ten, ten file bi trung trong trash

echo "=== B04: Ten file trung nhau khi trash nhieu file cung ten ==="
echo ""

# Chuan bi
rm -rf ~/.local/share/trash/*
mkdir -p /tmp/dir1 /tmp/dir2
echo "file 1" > /tmp/dir1/test.txt
echo "file 2" > /tmp/dir2/test.txt

echo "1. Tao file: /tmp/dir1/test.txt va /tmp/dir2/test.txt"
echo ""
echo "2. Trash file thu nhat: tr /tmp/dir1/test.txt"
../build/tr /tmp/dir1/test.txt
echo ""
echo "3. Trash file thu hai: tr /tmp/dir2/test.txt"
../build/tr /tmp/dir2/test.txt

echo ""
echo "4. Danh sach trash:"
../build/tr -l

echo ""
echo "5. Kiem tra ten file trung:"
find ~/.local/share/trash/ -name "test.txt" -type f

# Don dep
rm -rf /tmp/dir1 /tmp/dir2

echo ""
echo "=== Ket thuc B04 ==="
