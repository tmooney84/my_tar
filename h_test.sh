
#!/bin/bash

echo "Starting h_test.sh"

cd ~/Projects/my_tar/
# Build the project
make my_tar || exit 1

# Copy the built file
cp ~/Projects/my_tar/my_tar ~/Projects/my_tar/testing123/dir1

# Need to switch folders
cd ~/Projects/my_tar/testing123/dir1/ || exit 1

# Remove old tar files (if they exist)
rm -f my_test.tar test.tar

# Create new tar files
./my_tar -cf my_test.tar file1.txt file2.txt

tar -cf test.tar file1.txt file2.txt

echo "test.tar output:"
cat test.tar || exit 1
echo "my_test.tar output:"
cat my_test.tar || exit 1

# Return to the original directory
cd ~/Projects/my_tar || exit 1

echo "Finished copying and updating files"

