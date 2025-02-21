
#!/bin/bash

echo "Starting h_test.sh"

cd ~/Projects/my_tar/
# Build the project
make my_tar || exit 1

# Copy the built file
cp ~/Projects/my_tar/my_tar ~/Projects/temp_stuff/testing123/dir1

# Need to switch folders
cd ~/Projects/temp_stuff/testing123/dir1/ || exit 1

# Remove old tar files (if they exist)
rm -f my_test.tar test.tar

# Create new tar files
./my_tar -cf my_test.tar file1.txt file2.txt

tar -cf test.tar file1.txt file2.txt


# Compare tar outputs
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

# Return to the original directory
cd ~/Projects/my_tar/ || exit 1

echo "Finished copying and updating files"

