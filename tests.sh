
#!/bin/bash

echo "Starting tests.sh..."


# Build the project
make my_tar || exit 1

echo ""
echo "------------------------------------------------------------------"

mkdir test_dir

# Copy the built file
cp ./my_tar ./test_dir || exit 1
cd test_dir || exit 1

# create test1.txt and test2.txt for testing
echo "1234567890" > file1.txt
echo "abcdefghij" > file2.txt

# Create new tar files
./my_tar -cf my_test.tar file1.txt file2.txt

tar -cf test.tar file1.txt file2.txt


# Test 1: Compare create tar outputs of two basic files
echo "TEST 1: tar -cf my_test.tar file1.txt file2.txt   -vs-   tar version"
echo ""
echo "hexdump -C test.tar vs. my_test.tar"
diff <(hexdump -C test.tar) <(hexdump -C my_test.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -c output:"
diff <(hexdump -c test.tar) <(hexdump -c my_test.tar)

echo ""

echo "test.tar output:"
cat test.tar || exit 1
echo "my_test.tar output:"
cat my_test.tar || exit 1


# Test 2: Compare create tar outputs of two basic files and directory

mkdir dir1
cd dir1

echo "0987654321" > txtdir1.txt
echo "ABCDEFGHIJ" > txtdir2.txt

cd ..

# Create new tar files      !!! Should overwrite orginal tests
./my_tar -cf my_test.tar file1.txt file2.txt dir1

tar -cf test.tar file1.txt file2.txt dir1


# Compare tar outputs

echo ""

echo "------------------------------------------------------------------"

echo "TEST 2: tar -cf my_test.tar file1.txt file2.txt dir1   -vs-   tar version"

echo "hexdump -C test.tar vs. my_test.tar"
diff <(hexdump -C test.tar) <(hexdump -C my_test.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -c output:"
diff <(hexdump -c test.tar) <(hexdump -c my_test.tar)

echo ""

echo "test.tar output:"
cat test.tar || exit 1
echo "my_test.tar output:"
cat my_test.tar || exit 1


#  return to orginal directory
cd .. || exits 1

rm file_header_fns.o main.o my_printf.o my_tar print_error.o utils.o

rm -rf test_dir || exit 1