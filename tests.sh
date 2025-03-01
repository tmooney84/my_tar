
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
echo "diff hexdump -C test.tar vs. my_test.tar"
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

echo "diff hexdump -C test.tar vs. my_test.tar"
diff <(hexdump -C test.tar) <(hexdump -C my_test.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -c output:"
diff <(hexdump -c test.tar) <(hexdump -c my_test.tar)

echo ""

echo "test.tar output:"
cat test.tar || exit 1
echo "my_test.tar output:"
cat my_test.tar || exit 1


# Test 3: Compare create tar outputs of two basic files and directory

echo ""

echo "------------------------------------------------------------------"

echo "TEST 3: tar -xf test.tar"

mv file1.txt file1orig.txt || exit 1
mv file2.txt file2orig.txt || exit 1
mv dir1 dir1orig || exit 1

# Create new tar files      !!! Should overwrite orginal tests
#./my_tar -cf my_test.tar file1.txt file2.txt dir1

#tar -cf test.tar file1.txt dir1 not_file.txt

./my_tar -xf test.tar
#./my_tar -xf test.tar file1.txt dir1 not_file.txt

# tar output


echo "diff hexdump -C file1.txt vs. original"
diff <(hexdump -C file1.txt) <(hexdump -C file1orig.txt) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -C file2.txt vs. original"
diff <(hexdump -C file2.txt) <(hexdump -C file2orig.txt) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -C dir1 vs dir1orig output:"
diff <(hexdump -C dir1) <(hexdump -C dir1orig)

echo ""

echo "diff hexdump -C txtdir1.txt vs txtdir1.txt (original) output:"
diff <(hexdump -C dir1/txtdir1.txt) <(hexdump -C dir1orig/txtdir1.txt)

echo ""

echo "diff hexdump -C dir1 vs dir1orig output:"
diff <(hexdump -C dir1/txtdir2.txt) <(hexdump -C dir1orig/txtdir2.txt)

echo ""

echo "file1.txt output:"
cat file1.txt || exit 1

echo "file1org.txt output:"
cat file1org.txt || exit 1

echo "file2.txt output:"
cat file2.txt || exit 1

echo "file2org.txt output:"
cat file2org.txt || exit 1





#  return to orginal directory
cd .. || exits 1

rm file_header_fns.o main.o my_printf.o my_tar print_error.o utils.o

rm -rf test_dir || exit 1