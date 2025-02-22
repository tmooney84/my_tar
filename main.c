/*
particular circumstances:
$ tar -c file1.c file2.txt dir3
>>tar: Refusing to write archive contents to terminal (missing -f option?)
>>tar: Error is not recoverable: exiting now
    these two lines are the response

  tar -cf
  tar -rf
  tar -tf
  tar -xf

  tar -tf -cf arc1
tar: -cf: Cannot open: No such file or directory
tar: Error is not recoverable: exiting now

needs to be a flags for c,r,t,x and f, + v flag. If one of those flags triggered the remainder of
args are files/directories ...except build in functionality for compression
gzip is a very common and fast compression algorithm.


gzip: tar -czvf my_archive.tar.gz my_directory
bzip2 generally provides better compression than gzip but is slower.
bzip2: tar -cjvf my_archive.tar.bz2 my_directory
//create my own compression later?

Exit Status >>> return 0 on success >0 if an error occurs (return 1)

Errors handling Errors will be written on STDERR.
File not found (provided file is: i_don_t_exist): my_tar: i_don_t_exist: Cannot stat: No such file or directory

Error with the tarball file (provided file is: tarball.tar): my_tar: Cannot open tarball.tar
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include "utils.h"
#include "my_printf.h"
#include "print_error.h"
#include "file_header_fns.h"

#define RECORDSIZE 20
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32
#define PREFIXSIZE 155
#define BLOCKSIZE 512

#define PATH_MAX 4096
// better to use a union? padding needed?

#define TAR_PERMS 0664 // RW for owner, group & R for others

#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2


void flag_error();
void file_error(char *file_name);
void failed_alloc();
void print_string_array(char **all_names, int num_names);
void free_string_array(char **names, int num_names);
header *fill_header_info(char *file);

char **create_names_array(int argc, char **argv, int num_names);

int create_file(char *file_name, int flags, int perms);
int create_tar(char **names, int num_names); // int v_flag
int create_tar_file(char *tar_name);
int append_file_data(int tar_fd, char *append_file);
int update_tar(int argc, char **argv);
int list_tar(int argc, char **argv, int v_flag);
int extract_tar(char **names, int num_names); // int v_flag
int process_entry(char *path, int tar_fd);
int write_header(header *hdr, int tar_fd);
int write_file_data(int tar_fd, int f_fd, int f_size);
int write_padding(int tar_fd, int total_required_padding);

// tar -czf -t >>> will throw error
// parse the files but if already c_flag, etc. is 1 then file_error(argv[i])
// if t then cannot have c, r, t, u, x ... but needs f
// -f to parse file correctly

int main(int argc, char **argv)
{
    // c,f,r,t,u,x            + v, z, j flags
    // int c_flag = 0;
    // int f_flag = 0;
    // int r_flag = 0;
    // int t_flag = 0;
    // int u_flag = 0;
    // int x_flag = 0;

    // int v_flag = 0;
    // int z_flag = 0;
    // int j_flag = 0;
    int num_flag_args = 1; // for now is 1 to make it work

    int num_names = argc - (num_flag_args + 1);
    char **first_name = argv;
    char **names = create_names_array(argc, first_name, num_names);
    if (!names)
    {
        failed_alloc();
        return -1;
    }

    if (argc == 1)
    {
        flag_error();
        return -1;
    }
    //*********************need to count the number of dashes and redo this section!!! tar -c -f  versus  tar -cf name.tar file_name
    else if (my_strcmp(argv[1], "-cf") == 0)
    {
        // int fd = create_file("zzz.txt", O_CREAT | O_RDWR, 0664);
        int fd = create_tar(names, num_names);
        if(fd < 0)
        {
            print_error("Error creating tar file");
            return -1;
        } 
        
        //printf("fd successful if 0: %d\n", fd);
        // if (create_tar(names, num_names) == -1) //int v_flag
        // {
        //     return -1;
        // }

        // TEST file_header_fns.c IN main:
        // tester_main(argv[2]);
        return 0;
    }
    // else if(my_strcmp(argv[1], "-cvf")==0)
    //     {
    //         v_flag = 1;
    //         create_tar(argc, argv, v_flag);
    //     }
    else if (my_strcmp(argv[1], "-rf") == 0)
    {
        // archive_tar(argc, argv);
    }
    else if (my_strcmp(argv[1], "-tf") == 0)
    {
        //t_flag = 1;
        //f_flag = 1;
    }
    // else if(my_strcmp(argv[1], "-tvf")==0)
    //     {
    //         t_flag = 1;
    //         v_flag = 1;
    //         f_flag = 1;
    //     }
    else if (my_strcmp(argv[1], "-uf") == 0)
    {
        //u_flag = 1;
        //f_flag = 1;
    }
    else if (my_strcmp(argv[1], "-xf") == 0)
    {
        //extract_tar(names);
    }
    // else if(my_strcmp(argv[1], "-xvf")==0)
    //     {
    //         x_flag = 1;
    //         v_flag = 1;
    //         f_flag = 1;
    //     }

    else
    {
        flag_error();
        return -1;
    }

    //my_printf("argc: %d\n", argc);
   // my_printf("c: %d, f: %d, j: %d, r: %d, t: %d, u: %d, v: %d, x: %d, z: %d\n", c_flag, f_flag, j_flag, r_flag, t_flag, u_flag, v_flag, x_flag, z_flag);

    //*** NEED TO IMPLEMENT num_flag_args ABOVE  */

    // argv[2]
    // test whether string file or folder if not error

    //-cf -rf -uf      create archive? add to archive? add to archive if update (stat time conditional)? >>>
    // this could all be one fn w flags

    // print out archive contents >>> additional function

    // extract archive

    free_string_array(names, num_names);
    return 0;
}

void flag_error()
{
    print_error("tar: You must specify one of the '-Acdtrux', '--delete' or '--test-label' options\nTry 'tar --help' or 'tar --usage' for more information.");
}

void file_error(char *file_name)
{
    print_error("my_tar: %s: Cannot stat: No such file or directory", file_name);
}

void failed_alloc()
{
    print_error("Failed to allocate memory.\n");
}

void append_error()
{
    print_error("Failure to append file.\n");
}

void tarball_error(char *tar_name)
{
    my_printf("my_tar: Cannot open %s\n", tar_name);
}

void print_string_array(char **all_names, int num_names)
{
    for (int i = 0; i < num_names; i++)
    {
        my_printf("%s\n", all_names[i]);
    }
}

// frees pointers related to string array
void free_string_array(char **names, int num_names)
{
    if (names == NULL)
    {
        return;
    }

    for (int i = 0; i < num_names; i++)
    {
        free(names[i]); // Free each dynamically allocated string
    }

    free(names);
}

char **create_names_array(int argc, char **argv, int num_names)
{
    char **names = malloc((num_names) * sizeof(char *));
    if (!names)
    {
        failed_alloc();
        return NULL;
    }
    my_memset(names, 0, (num_names) * (sizeof(char *)));
    int names_index = 0;

    for (int i = argc - num_names; i < argc; i++)
    {
        int names_len = my_strlen(argv[i]);
        names[names_index] = malloc((my_strlen(argv[i]) + 1) * sizeof(char));
        if (!names[names_index])
        {
            failed_alloc();
            return NULL;
        }
        my_memset(names[names_index], 0, names_len);

        my_strncpy(names[names_index], argv[i], names_len);
        names_index++;
    }

    //print_string_array(names, names_index);
    return names;
}
// 346-416 commented for testing

int create_file(char *file_name, int flags, int perms)
{
    int fd;
    fd = open(file_name, flags, perms); // O_RDWR
    if (fd < 0)
    {
        tarball_error(file_name);
        return -1;
    }
    return fd;
}

int create_tar_file(char *tar_name)
{
    int tar_fd;

    // create tar file:
    tar_fd = create_file(tar_name, O_RDWR | O_CREAT | O_APPEND, TAR_PERMS);
    // tar_fd = create_file(tar_name, O_CREAT, TAR_PERMS);//TAR_PERMS
    //printf("tar_fd: %d\n", tar_fd);
    if (tar_fd < 0)
    {
        tarball_error(tar_name);
        return -1;
    }
    return tar_fd;
}

int create_tar(char **names, int num_names) // int v_flag
{
    int tar_fd;

    char *tar_name = names[0];
    //printf("tar_name: %s\n", tar_name);
    tar_fd = create_tar_file(tar_name);

    if (tar_fd < 0)
    {
        return -1;
    }

    for (int i = 1; i < num_names; i++)
    {
        //printf("test\n");
        if (process_entry(names[i], tar_fd) < 0)
        {
            my_printf("Error processing %s into tar file\n", names[i]);
            return 1;
        }
    }

    // add_zeros(tar_fd);

    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;
    //printf("tar_size before padding: %ld\n", tar_size);

    // need two zero blocks and then need to see if that goes over the size of a record
    int zero_padding = 2 * BLOCKSIZE;
    int padded_data = tar_size + zero_padding;
    int total_required_padding;

    int rec_num = (tar_size % (RECORDSIZE * BLOCKSIZE) == 0) ? tar_size / (RECORDSIZE * BLOCKSIZE) : tar_size / (RECORDSIZE * BLOCKSIZE) + 1;

    int rec_num_wpad = ((padded_data) % (RECORDSIZE * BLOCKSIZE) == 0) ? padded_data / (RECORDSIZE * BLOCKSIZE) : padded_data / (RECORDSIZE * BLOCKSIZE) + 1;

    if (rec_num == rec_num_wpad)
    {
        total_required_padding = rec_num * (RECORDSIZE * BLOCKSIZE) - tar_size;
    }
    else
    {
        total_required_padding = rec_num_wpad * (RECORDSIZE * BLOCKSIZE) - tar_size;
    }

    //printf("padding needed: %d\n", total_required_padding);

    if (write_padding(tar_fd, total_required_padding) < 0)
    {
        print_error("Unable to add padding\n");
        return -1;
    }

    tar_size = (long int)tar_stats.st_size;
    //printf("tar_size after padding added: %ld\n", tar_size);

    close(tar_fd);

    return 0;
}

int process_entry(char *path, int tar_fd)
{
    struct stat arg_stats;
    if (stat(path, &arg_stats) < 0)
    {
        file_error(path);
    }
    //tester_main(path);
    header *hdr = fill_header_info(path);

    //printf("tar_fd: %d\n", tar_fd);
    if (!hdr)
    {
        file_error(path);
        return -1;
    }
    write_header(hdr, tar_fd);

    free(hdr);

    if (S_ISREG(arg_stats.st_mode))
    {
        // if file to append
        if (append_file_data(tar_fd, path) != 0)
        {
            file_error(path);
            return -1;
        }
    }

    else if (S_ISDIR(arg_stats.st_mode))
    {
        DIR *dir = opendir(path);
        if (!dir)
        {
            file_error(path);
            return -1;
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            if (my_strcmp(entry->d_name, ".") == 0 || my_strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            char rel_path[PATH_MAX];
            my_memset(rel_path, 0, PATH_MAX);

            int entry_name_len = my_strlen(entry->d_name);
            int path_len = my_strlen(path);

            my_strncpy(rel_path, path, path_len);
            rel_path[path_len] = '/';
            my_strncpy(rel_path + path_len + 1, entry->d_name, entry_name_len);

            //printf("Full Path Name: %s\n", rel_path);
            if (process_entry(rel_path, tar_fd) < 0)
            {
                print_error("Failure to process directory entries");
                closedir(dir);
                return -1;
            }
        }

        closedir(dir);

        //    if(v_flag) >>> to print the file that was added
        // {
        //     my_printf("%s\n", names[i]);
        // }
    }

    return 0;
}




//////////////////////////////////////////////////
int extract_tar(char **names, int num_names) // int v_flag
{
    int tar_fd;

    char *tar_name = names[0];
    //printf("tar_name: %s\n", tar_name);
    tar_fd = open(tar_name, O_RDONLY, TAR_PERMS);

    if (tar_fd < 0)
    {
        tarball_error(tar_name);
        return -1;
    }

    if(num_names > 1)
    {
    for (int i = 1; i < num_names; i++)
    {
        if (extract_process_entry(names[i], tar_fd) < 0)
        {
            my_printf("Error processing %s into tar file\n", names[i]);
            return 1;
        }
    }
    }
else{
    extract_all_contents(tar_fd);
    }

    close(tar_fd);

    return 0;
////////////////////////
}


int extract_all_contents(int tar_fd)
{

   struct stat tar_stats;
   if(fstat(tar_fd, &tar_stats) == -1)
   {
        print_error("Unable to stat tar");
        return -1;
   } 

   long int tar_size = (long int)tar_stats.st_size;
   int num_blocks = tar_size / BLOCKSIZE;
   if(tar_size / BLOCKSIZE != 0)
   {
        print_error("Error non-uniform tar size");
        return -1;
   }
   //for first block
   char magic_test[6];
   size_t n = 0;

   
    //makes sure tar_fd is at beginning of the file
   lseek(tar_fd, 0, SEEK_SET);

    unsigned char header_block[512];
   if(n = read(tar_fd, header_block, 512) < 0)
   {
        print_error("Unable to read magic tar file");
        return -1;
    }

    struct header *f_header = (struct header *)header_block;
   
    char file_name[NAMESIZE];
    char file_flags = O_RDWR | O_CREAT | O_TRUNC;
    int file_perms = parse_octal(f_header->mode);

    int fd = create_file(f_header->name, file_flags, f_header->mode);
    
    if(fill_file(tar_fd, fd) < 0)
    {
        print_error("Unable to extract file contents");
        return -1;
    }

    if(fd < 0)
    {
        my_printf("Unable to create %s", file_name);
        return -1;
    }

    char 
    //file or directory found


   }
   else
   {
    lseek(tar_fd, BLOCKSIZE-157, SEEK_CUR);
   }
    //need to get name and flagtype (for what type of file)
    //if file and if dir

    //file create file and get stat info set up

    //dir build dir

    //should this go in extract_process_entry...?
   }



   lseek(tar_fd, i * 512, SEEK_CUR);
   }

   for(int i; i < tar_size; i = i + BLOCKSIZE)
   {
    
    int end_data = lseek(tar_fd, 0, SEEK_SET);
    if(end_data < 0)
    {
        print_error("Unable to random access tar file");
        return -1;
    }
   
    }
}


int parse_octal(char * oct_string)
{

}

// int extract_process_entry(char *path, int tar_fd)
// {
//     //need to find file header by magic and see if
//     //name matches path, file or dir... if file extract_file(), if directory create dir
//     //move into and then create file

//     //if path create file with the name and copy into 
//     //file

//     write_header(hdr, tar_fd);

//     free(hdr);

//     if (S_ISREG(arg_stats.st_mode))
//     {
//         // if file to append
//         if (append_file_data(tar_fd, path) != 0)
//         {
//             file_error(path);
//             return -1;
//         }
//     }

//     else if (S_ISDIR(arg_stats.st_mode))
//     {
//         DIR *dir = opendir(path);
//         if (!dir)
//         {
//             file_error(path);
//             return -1;
//         }

//         struct dirent *entry;

//         while ((entry = readdir(dir)) != NULL)
//         {
//             if (my_strcmp(entry->d_name, ".") == 0 || my_strcmp(entry->d_name, "..") == 0)
//             {
//                 continue;
//             }

//             char rel_path[PATH_MAX];
//             my_memset(rel_path, 0, PATH_MAX);

//             int entry_name_len = my_strlen(entry->d_name);
//             int path_len = my_strlen(path);

//             my_strncpy(rel_path, path, path_len);
//             rel_path[path_len] = '/';
//             my_strncpy(rel_path + path_len + 1, entry->d_name, entry_name_len);

//             //printf("Full Path Name: %s\n", rel_path);
//             if (process_entry(rel_path, tar_fd) < 0)
//             {
//                 print_error("Failure to process directory entries");
//                 closedir(dir);
//                 return -1;
//             }
//         }

//         closedir(dir);

//         //    if(v_flag) >>> to print the file that was added
//         // {
//         //     my_printf("%s\n", names[i]);
//         // }
//     }

//     return 0;
// }



























int write_padding(int tar_fd, int total_required_padding)
{
    char zero_buff[total_required_padding];
    my_memset(zero_buff, 0, total_required_padding);

    ssize_t bytes_written = 0;

    int end_data = lseek(tar_fd, 0, SEEK_END);
    if(end_data < 0)
    {
        print_error("Unable to random access tar file");
        return -1;
    }
    // printf("End data: %d\n", end_data);

    while (bytes_written < total_required_padding)
    {
        ssize_t written = write(tar_fd, zero_buff + bytes_written, total_required_padding - bytes_written);
        if (written < 0)
        {
            print_error("Failure to write data.\n");
            return -1;
        }
        bytes_written += written;
        // printf("writing data\n");
    }
    // printf("bytes_written: %zu\n", bytes_written);
    return 0;
}

int append_file_data(int tar_fd, char *append_file)
{
    //printf("append_file_data named %s started!!!!!!\n", append_file);
    // get file size
    struct stat file_stats;

    // what to do about symbolic links lsat and in tar??
    if (stat(append_file, &file_stats) == -1)
    {
        return -1;
    }

    long int f_size = (long int)file_stats.st_size;
    //printf("f_size in append: %ld\n", f_size);

    // get tar size
    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;
    //printf("tar_size in append: %ld\n", tar_size);

    int append_fd = open(append_file, O_RDONLY);

    if (tar_size < BLOCKSIZE)
    {
        print_error("Failure to write file header");
        return -1;
    }

    // if the tar is larger than size of header then just append because
    else
    {
        int ts_n;

        ts_n = write_file_data(tar_fd, append_fd, f_size);

        if (ts_n < 0)
        {
            print_error("Unable to append file data\n");
            return -1;
        }
    }

    close(append_fd);
   // printf("closed append_fd\n");
    return 0; // if successful may need conditional logic
}

int write_header(header *hdr, int tar_fd)
{
    unsigned char *hdr_data = (unsigned char *)hdr;
    size_t bytes_written = 0;

    while (bytes_written < BLOCKSIZE)
    {
        // ssize_t >>> [-1, SIZE_MAX] bytes, if issue returns -1
        ssize_t written = write(tar_fd, hdr_data + bytes_written, BLOCKSIZE);
        if (written < 0)
        {
            print_error("write_header: write failed");
            return -1;
        }
        bytes_written += written;
    }
    //printf("bytes_written: %ld\n", bytes_written);
    //printf("header written\n");
    return 0;
}

int write_file_data(int tar_fd, int f_fd, int f_size)
{
   // printf("write_file_data starting...\n");
    unsigned char transfer_buff[BLOCKSIZE];
    ssize_t total_bytes_written = 0;
    ssize_t n = 0;
    // This logic is for the partial writes in the events of system buffering and interrupts... more common in pipes,fifos and sockets
    // than regular files, but can possibly happen
    while ((f_size <= 0 || total_bytes_written < f_size) &&
           (n = read(f_fd, transfer_buff, BLOCKSIZE)) > 0)
    {
        ssize_t bytes_written = 0;
        while (bytes_written < n)
        {
            ssize_t written = write(tar_fd, transfer_buff + bytes_written, n - bytes_written);
            if (written < 0)
            {
                print_error("Failure to write file data");
                return -1;
            }
            bytes_written += written;
        }
        total_bytes_written += n;
    }

    if (n < 0)
    {
        print_error("read error");
        return -1;
    }
    // need to write additional bytes to fill data block
    //printf("bytes_written: %ld\n", total_bytes_written);

    ssize_t additional_size;
    int add_written;

    if (total_bytes_written % BLOCKSIZE != 0)
    {
        additional_size = BLOCKSIZE - total_bytes_written % BLOCKSIZE;
    }

    if(additional_size > 0)
{
    unsigned char add_buff[additional_size];
    my_memset(add_buff, '\0', additional_size);

    add_written = write(tar_fd, add_buff, additional_size);

    if (add_written != additional_size)
    {
        print_error("Failure to write file data intrablock padding");
        return -1;
    }
 
}
   
    //printf("additional bytes_written: %d\n", add_written);
   // printf("GRAND TOTAL FOR FILE bytes_written: %ld\n", total_bytes_written + add_written);


// struct stat tar_stats;

// if (fstat(tar_fd, &tar_stats) == -1)
// {
//     return -1;
// }

// long int tar_size = (long int)tar_stats.st_size;
// printf("tar_size in write_file_data: %ld\n", tar_size);

return 0;
}

//        //check if is file vs dir
//        file_header_info(names[i]);

//        if(names[i] filetype is a file)
//        {
//            append_file();  >>> includes  fill_header(names[i]);>>> MAKE SURE TO START APPENDING WRITE WHEN ZERO PADDING STARTS AND MAKE

//            ADJUSTMENTS ACCORDINGLY
//        }
//        //if dir then need to go in recursively
//        {
//        recursive into directory and sub-directories
//        {

//        if(names[i] filetype is a direcory)
//                    (recursively for files in folders and sub_folders)
//        {
//            file_header_info(directory_name);

//            do the same for each file in the directory... recursively
//            file_header_info(file_name);
//            append_file() >>> for each file in the directories
//        }
//         }
//            }
//    }

//    //add two zero blocks to end of file >>> this could be part of the append_file function at end
//    add_zero_block(tar_file_name.tar);
//    add_zero_block(tar_file_name.tar);
//    logic to make sure that the bocks and records align properly

//

/*
determine total size of files + size of headers + 2 "00" 512bytes with padding to determine
how many records are needed (20 blocks/ record)

1) -create file with the supplied name
function: create_file(char *file_name)

could be more efficient if do a first sweep through gathering the data and building temporary
tars

file 1 ... file 2 ... file 3

-get info and fill header for file 1, same for file2 and file3... take the size field convert
to int (size1 + size2 + size3 + 3 x FH_SIZE is this < 20 blocks use % and if not exact fix need pad the
last record and then an addional 2 512 "00" blocks into an additional record)

sub-functions: fill_header_info(char * file_name) >>> need think about how this will be run recursively for folders
           calc_num_records (int file_sizes[argc - non_file_args]) >>> remember int size = sizeof(myArray) / sizeof(myArray[0]);
           add_zero_block(tar_file_name.tar);
           append_file(char *names)



challenges: need to research how to do this recursively for folders

2) after gathering file info into header blocks begin to copy the file byte-by-byte
into each block until filled, if not exact fit into last block of record >>> add padding

sub-functions:
append_tar(char *tar_file, char *append_file_name) >>> can reuse with update_file fn, just need to include
-r flag or do quazi overload... update_file_name >>> same logic just with -r to do conditional on update

verbose printing of file completed need to test -v

add zero blocks record to end of file
^^^ this may be a sub-function

verbose printing of tar completed >>> need to test -v

return 0 success 1 for failure
*/

// int archive_tar(int argc, char **argv)

// // append_tar(char **names, int num_names) >>> first of the list should be the tar file
// {
//     /*
//     append_file(char **names, int num_names); >>> includes the 2 x zero blocks
//     //add two zero blocks to end of file >>> this could be part of the append_file function at end
//     add_zero_block(tar_file_name.tar);
//     add_zero_block(tar_file_name.tar);
//     logic to make sure that the bocks and records align properly
// }

// int update_tar(int argc, char **argv)
// {
// if(new_file.modification_time > old_file.modification_time)
// {
// append_file() >>> with same logic as append_tar
// }
// }
// update/append_file(char *tar_file, char *append_file_name) >>> can reuse with update_file fn, just need to include
// -r flag or do quazi overload... update_file_name >>> same logic just with -r to do conditional on update

// */
// }

// int list_tar(int argc, char **argv, int v_flag)
// {
//     /*cycle through testing whether type is file or directory, if directory add '/' to end when
//     print to screen; rely '/' on the dir1/file3.txt to skip to know
//     whether it is a file or a path
//     */
// }

// int extract_tar(int argc, char **argv, int v_flag)
// {
//     /*
//     cycle through file to find headers until hit the two zero blocks
//        until end of tar
//         if file: create(overwrite?) file after fast forwarding through to file beginning in tar and
//         run the copy as far as the end of the archived file

//         if dir: create dir with header ...contained files should be named "dir1/file1.txt" so should
//         picked up by the previous logic
//     */
// }

// // adding verbose to these just have a print and with ifv_flag)

// // zip would be additional logic>>> some form of api
