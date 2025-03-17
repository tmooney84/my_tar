#!/bin/bash

tar -cf dir.tar dir1
./my_tar -cf my_dir.tar dir1

echo "tar -cf dir.tar  dir1"
echo "./my_tar -cf my_dir.tar dir1"
echo "---------------------------------------------------------------------------"
echo "---------------------------------------------------------------------------"
echo hexdump diffs:
echo ""
echo "hexdump -C dir.tar vs. my_dir.tar"
echo ""
echo ""
diff <(hexdump -C dir.tar) <(hexdump -C my_dir.tar) && echo "Files match!" || echo "Files differ!"

echo ""
echo ""
diff <(hexdump -C dir.tar) <(hexdump -C my_dir.tar) && echo "Files match!" || echo "Files differ!"

echo ""
echo "---------------------------------------------------------------------------"
echo "---------------------------------------------------------------------------"

echo ""

echo "dir.tar output:"
echo ""
cat dir.tar || exit 1
echo ""
echo ""
echo "my_dir.tar output:"
echo ""
cat my_dir.tar || exit 1

echo ""

echo ""

echo ""





echo ""

echo ""

echo ""
echo "***********************************************************************************"
echo "***********************************************************************************"
echo "dir.tar output:"
echo ""
hexdump -c dir.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"
echo "my_dir.tar output:"
echo ""
hexdump -c my_dir.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"

echo ""
echo ""
echo ""










echo "***********************************************************************************"
echo "***********************************************************************************"
echo "dir.tar output:"
echo ""
hexdump -C dir.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"
echo "my_dir.tar output:"
echo ""
hexdump -C my_dir.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"

echo ""
echo ""
echo ""
echo "dir.tar output:"
echo ""
cat dir.tar || exit 1

echo ""
echo ""

echo "my_dir.tar output:"
echo ""
cat my_dir.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"

echo ""