# Welcome to My Tar
***
This is a hand-built Linux/Posix-compatible Utility that is similar to Linux tape archive (tar)

## Task
my_tar archives files and directories in a GNU-tar format. It provides similar functionality to tar command in relation to create (-cf), extract (-xf), list(-tf), append(-rf) and update(-uf) functionality.

## Description
my_tar attempts to use a modular approach to provide create, extract, list, append and update functionality similar to tar. It uses a library of functions built primarily using basic Linux system calls. The update functionality (-uf) uses a custom hash table for tracking the usage and existance of command line file names to provide for faster execution of -uf. 

## Installation
In order to use my_tar, the program needs to be cloned from its Gitea Repository, and then the source code needs to be compiled locally.

Clone the my_tar repository into current directory: 
```
git clone git@git.us.qwasar.io:my_tar_178978_gparg-/my_tar.git
```

Move into the my_tar directory:
```
cd my_tar
```

Compile the executable program to run locally in the contained folder:
```
make my_tar
```

## Usage
Create tar file:

```
./my_tar -cf <tarfile_name> <file_to_add> ... 
```

List tar file contents:

```
./my_tar -tf <tarfile_name>
```

Append tar file contents:

```
./my_tar -rf <tarfile_name> <file_to_append> ...
```

Update tar file contents (dependent on if file(s) to be added have a newer modified time than files with the identical file name 
already included in the tar file):

```
./my_tar -uf <tarfile_name> <file_to_add> ...
```


## Future Improvements
1) Add the -v flag and verbose functionality
2) Add compression support for .tar.gz files (gzip compression) using zlib libraries
3) Develop hand-rolled Huffman Coding and Data Compression to implement
4) Clean up coding logic to handle edge cases



### The Core Team


<span><i>Made at <a href='https://qwasar.io'>Qwasar SV -- Software Engineering School</a></i></span>
<span><img alt='Qwasar SV -- Software Engineering School's Logo' src='https://storage.googleapis.com/qwasar-public/qwasar-logo_50x50.png' width='20px' /></span>
