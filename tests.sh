#!/bin/bash

echo "Starting tests.sh..."


# Build the project
make my_tar || exit 1

echo ""
echo "------------------------------------------------------------------"
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
echo ""
diff <(hexdump -C test.tar) <(hexdump -C my_test.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -c output:"
echo ""
diff <(hexdump -c test.tar) <(hexdump -c my_test.tar)

echo ""

echo "test.tar output:"
echo ""
cat test.tar || exit 1
echo "my_test.tar output:"
echo ""
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
echo "------------------------------------------------------------------"

echo "TEST 2: tar -cf my_test.tar file1.txt file2.txt dir1   -vs-   tar version"

echo ""
echo "diff hexdump -C test.tar vs. my_test.tar"
echo ""
diff <(hexdump -C test.tar) <(hexdump -C my_test.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -c output:"
echo ""
diff <(hexdump -c test.tar) <(hexdump -c my_test.tar)

echo ""

echo "test.tar output:"
echo ""
cat test.tar || exit 1
echo "my_test.tar output:"
echo ""
cat my_test.tar || exit 1


# Test 3: Compare create tar outputs of two basic files and directory

echo ""

echo "------------------------------------------------------------------"
echo "------------------------------------------------------------------"

echo "TEST 3: tar -xf test.tar"
echo ""

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
echo ""
diff <(hexdump -C file1.txt) <(hexdump -C file1orig.txt) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -C file2.txt vs. original"
echo ""
diff <(hexdump -C file2.txt) <(hexdump -C file2orig.txt) && echo "Files match!" || echo "Files differ!"

echo ""

# echo "diff hexdump -C dir1 vs dir1orig output:"
# diff <(hexdump -C dir1) <(hexdump -C dir1orig)

ls -l

echo ""

echo "diff hexdump -C txtdir1.txt vs txtdir1.txt (original) output:"
echo ""
diff <(hexdump -C dir1/txtdir1.txt) <(hexdump -C dir1orig/txtdir1.txt) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -C dir1 vs dir1orig output:"
echo ""
diff <(hexdump -C dir1/txtdir2.txt) <(hexdump -C dir1orig/txtdir2.txt) && echo "Files match!" || echo "Files differ!"

echo ""

#cd ..

echo "file1.txt output:"
echo ""
cat file1.txt || exit 1

# echo "file1org.txt output:"
# cat file1org.txt || exit 1

echo "file2.txt output:"
echo ""
cat file2.txt || exit 1

# echo "file2org.txt output:"
# cat file2org.txt || exit 1

echo ""

echo "------------------------------------------------------------------"
echo "------------------------------------------------------------------"

echo "TEST 4: tar -tf test.tar"

./my_tar -tf test.tar | tee my_printtar.txt

tar -tf test.tar | tee printtar.txt

echo "diff ./my_tar -tf test.tar vs. tar -tf test.tar"
echo ""
diff my_printtar.txt printtar.txt && echo "Files match!" || echo "Files differ!"

rm my_printtar.txt printtar.txt

echo "------------------------------------------------------------------"
echo "------------------------------------------------------------------"

echo "TEST 5: tar -tf test.tar with functional additional args"

./my_tar -tf test.tar file1.txt | tee my_printtar.txt

tar -tf test.tar file1.txt | tee printtar.txt

echo "diff ./my_tar -tf test.tar file1.txt vs. tar -tf test.tar"
echo ""
diff my_printtar.txt printtar.txt && echo "Files match!" || echo "Files differ!"

rm my_printtar.txt printtar.txt

echo "------------------------------------------------------------------"
echo "------------------------------------------------------------------"

echo "TEST 6: tar -tf test.tar with functional and non-functional additional args"

echo ""

echo "./my_tar -tf test.tar file1.txt wrongfile.txt:"
echo ""
./my_tar -tf test.tar file1.txt wrongfile.txt | tee my_printtar.txt

echo ""

echo "tar -tf test.tar file1.txt wrongfile.txt:"
echo ""
tar -tf test.tar file1.txt wrongfile.txt | tee printtar.txt

echo ""

echo "diff ./my_tar -tf test.tar file1.txt vs. tar -tf test.tar"
echo ""
diff my_printtar.txt printtar.txt && echo "Files match!" || echo "Files differ!"

rm my_printtar.txt printtar.txt

echo "------------------------------------------------------------------"
echo "------------------------------------------------------------------"

echo "TEST 7: tar -rf arch.tar vs. my_arch.tar"

echo "1234567890" > reg1.txt

echo "abcdefghij" > reg2.txt

mkdir reg

echo "0987654321" > reg/test1.txt
echo "jighfedcba" > reg/test2.txt

echo "additional file" > add1.txt

echo "TEST 7: tar -rf my_arch.tar add1.txt  -vs-   tar version"

# tar -cf my_arch.tar reg1.txt reg2.txt reg  ### normal tar version

###
./my_tar -cf my_arch.tar reg1.txt reg2.txt reg

tar -cf arch.tar reg1.txt reg2.txt reg

###
./my_tar -rf my_arch.tar add1.txt

tar -rf arch.tar add1.txt

echo "diff hexdump -C arch.tar vs. my_arch.tar"
echo ""
diff <(hexdump -C arch.tar) <(hexdump -C my_arch.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "diff hexdump -c output:"
echo ""
diff <(hexdump -c arch.tar) <(hexdump -c my_arch.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo ""
echo ""
echo "***********************************************************************************"
echo "arch.tar output:"
echo ""
hexdump -C arch.tar || exit 1

echo ""
echo ""
echo ""
echo "***********************************************************************************"
echo "my_arch.tar output:"
echo ""
hexdump -C my_arch.tar || exit 1

echo "***********************************************************************************"

echo ""
echo ""
echo ""
echo "arch.tar output:"
echo ""
cat arch.tar || exit 1

echo "my_arch.tar output:"
echo ""
cat my_arch.tar || exit 1


###
rm reg1.txt reg2.txt add1.txt

###
rm -rf reg



########################################################3
echo ""
echo ""

echo "***********************************************************************************"
echo "***********************************************************************************"

echo ""
echo "TEST 8: tar -uf vs. ./my_tar -uf"

echo ""
echo "Existing files in update.tar and my_update.tar: file1.txt file2.txt"
echo "Files to update: file1.txt file2.txt file3.txt dir4 file3.txt wrong_file.txt"
echo "***Note that:" 
echo "file1.txt has the same modification time"
echo "file2.txt now has an older modification time"
echo "file3.txt is duplicated in prompt"
echo "wrong_file.txt does not exist"
echo ""
echo "!!!The only two entries that should be added to tar are dir4 and file3.txt"

echo ""
echo ""

mkdir update_dir 

cp my_tar update_dir

cd update_dir

echo "0123456789" > file1.txt
echo "abcdefghij" > file2.txt
echo "this is file 3!!!" > file3.txt

mkdir dir4

tar -cf update.tar file1.txt file2.txt
./my_tar -cf my_update.tar file1.txt file2.txt 

touch -r ../../README.md file2.txt # change the modification time of file2.txt to an older value

tar -uf update.tar file1.txt file2.txt file3.txt dir4 file3.txt wrong_file.txt
./my_tar -uf my_update.tar file1.txt file2.txt file3.txt dir4 file3.txt wrong_file.txt

echo ""
echo ""

echo "***********************************************************************************"
echo "diff hexdump -C update.tar vs. my_update.tar"
echo ""
diff <(hexdump -C update.tar) <(hexdump -C my_update.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo "***********************************************************************************"
echo "diff hexdump -c output:"
echo ""
diff <(hexdump -c update.tar) <(hexdump -c my_update.tar) && echo "Files match!" || echo "Files differ!"

echo ""

echo ""

echo ""
echo "***********************************************************************************"
echo "update.tar output:"
echo ""
hexdump -C update.tar || exit 1

echo ""
echo ""
echo ""
echo "***********************************************************************************"
echo "my_update.tar output:"
echo ""
hexdump -C my_update.tar || exit 1

echo "***********************************************************************************"

echo ""
echo ""
echo ""
echo "update.tar output:"
echo ""
cat update.tar || exit 1

echo ""
echo ""

echo "my_update.tar output:"
echo ""
cat my_update.tar || exit 1

echo ""
echo ""
echo "***********************************************************************************"

echo ""

cd ..

echo "Finished testing..."





#  return to orginal directory
cd .. || exits 1

rm file_header_fns.o main.o my_printf.o my_tar print_error.o utils.o tar_utils.o hashmap.o

###
rm -rf test_dir || exit 1