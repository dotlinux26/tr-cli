#!/bin/bash
# B03: Loi khi trash file trong thu muc khong co quyen ghi
# Anh: screenshots/B03_permission.png
# Mo ta: Loi khi thuc hien trash file trong thu muc khong co quyen ghi

echo "=== B03: Loi khi trash file trong thu muc khong co quyen ghi ==="
echo ""

# Chuan bi
rm -rf ~/.local/share/trash/*
mkdir -p /tmp/testdir_perm
touch /tmp/testdir_perm/testfile.txt

echo "1. Tao thu muc: mkdir -p /tmp/testdir_perm"
echo "2. Tao file: touch /tmp/testdir_perm/testfile.txt"
echo "3. Gan quyen: chmod 444 /tmp/testdir_perm"
chmod 444 /tmp/testdir_perm

echo ""
echo "4. Chay: tr /tmp/testdir_perm/testfile.txt"
../build/tr /tmp/testdir_perm/testfile.txt
EXIT_CODE=$?

echo ""
echo "5. Ma loi thoat: $EXIT_CODE"

# Dọn dẹp
chmod 755 /tmp/testdir_perm
rm -rf /tmp/testdir_perm

echo ""
echo "=== Ket thuc B03 ==="
