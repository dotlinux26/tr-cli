#!/bin/bash
# B02: Crash khi chay tr voi file khong ton tai
# Anh: screenshots/B02_crash.png
# Mo ta: Chuong trinh crash voi segmentation fault

echo "=== B02: Crash khi chay tr voi file khong ton tai ==="
echo ""

echo "1. Chay: tr file_khong_ton_tai.txt"
../build/tr file_khong_ton_tai.txt
EXIT_CODE=$?

echo ""
echo "2. Ma loi thoat: $EXIT_CODE"
if [ $EXIT_CODE -eq 139 ]; then
    echo "CHUONG TRINH CRASH (segfault)!"
elif [ $EXIT_CODE -ne 0 ]; then
    echo "Chuong trinh thoat voi ma loi $EXIT_CODE (dung)"
else
    echo "Chuong trinh chay binh thuong"
fi

echo ""
echo "=== Ket thuc B02 ==="
