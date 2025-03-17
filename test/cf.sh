#!/bin/bash
tar -cf tester.tar file1.txt file2.txt dir1
./mytar -cf my_tester.tar file1.txt file2.txt dir1

echo "tar -cf tester.tar file1.txt file2.txt dir1"
echo "./mytar -cf my_tester.tar file1.txt file2.txt dir1"
echo "---------------------------------------------------------------------------"
echo "---------------------------------------------------------------------------"
echo hexdump diffs:
echo ""
echo "hexdump -C tester.tar vs. my_tester.tar"
echo ""
echo ""
diff <(hexdump -C tester.tar) <(hexdump -C my_tester.tar) && echo "Files match!" || echo "Files differ!"

echo ""
echo ""
diff <(hexdump -C tester.tar) <(hexdump -C my_tester.tar) && echo "Files match!" || echo "Files differ!"

echo ""
echo "---------------------------------------------------------------------------"
echo "---------------------------------------------------------------------------"

echo ""

echo "tester.tar output:"
echo ""
cat tester.tar || exit 1
echo ""
echo ""
echo "my_tester.tar output:"
echo ""
cat my_tester.tar || exit 1

echo ""

echo ""

echo ""
echo "***********************************************************************************"
echo "***********************************************************************************"
echo "tester.tar output:"
echo ""
hexdump -C tester.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"
echo "my_tester.tar output:"
echo ""
hexdump -C my_tester.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"

echo ""
echo ""
echo ""
echo "tester.tar output:"
echo ""
cat tester.tar || exit 1

echo ""
echo ""

echo "my_tester.tar output:"
echo ""
cat my_tester.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"

echo ""