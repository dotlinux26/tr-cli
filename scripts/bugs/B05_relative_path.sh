#!/bin/bash
# B05: Khong xu ly duong dan tuong doi
# Anh: screenshots/B05_relative_path.png
# Mo ta: Chuong trinh khong xu ly dung duong dan tuong doi (./, ../)

echo "=== B05: Khong xu ly duong dan tuong doi ==="
echo ""

# Chuan bi
rm -rf ~/.local/share/trash/*
cd /tmp
touch testfile_relative.txt

echo "1. Tao file: /tmp/testfile_relative.txt"
echo "2. Chay: tr ./testfile_relative.txt"
../build/tr ./testfile_relative.txt
EXIT_CODE=$?

echo ""
echo "3. Ma loi thoat: $EXIT_CODE"

echo ""
echo "4. Kiem tra file:"
if [ -f /tmp/testfile_relative.txt ]; then
    echo "File van ton tai tai: /tmp/testfile_relative.txt"
else
    echo "File da bi di chuyen"
fi

echo ""
echo "5. Danh sach trash:"
../build/tr -l

# Don dep
cd /home/nguyenduccanh/trashcli/tr-cli/scripts/bugs
rm -f /tmp/testfile_relative.txt

echo ""
echo "=== Ket thuc B05 ==="
