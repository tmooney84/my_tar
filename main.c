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

#include "utils.h"
#include "my_printf.h"
#include "print_error.h"

#define RECORDSIZE 512
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32

// better to use a union? padding needed?
typedef struct header
{                        /* byte offset */
    char name[NAMESIZE]; /*   0 */
    char mode[8];        /* 100 */
    char uid[8];         /* 108 */
    char gid[8];         /* 116 */
    char size[12];       /* 124 */
    char mtime[12];      /* 136 */
    char chksum[8];      /* 148 */
    char typeflag;       /* 156 */
    char linkname[100];  /* 157 */
    char magic[6];       /* 257 */
    char version[2];     /* 263 */
    char uname[TUNMLEN]; /* 265 */
    char gname[TGNMLEN]; /* 297 */
    char devmajor[8];    /* 329 */
    char devminor[8];    /* 337 */
    char prefix[155];    /* 345 */
                         /* 500 */
    char padding[12];    // POTENTIAL PADDING
} header;

typedef struct block
{
    char content[512];
} block;

#define TAR_PERMS 0664 // RW for owner, group & R for others

#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE '0'   /* regular file */
#define AREGTYPE '\0' /* regular file */
#define LNKTYPE '1'   /* link */
#define SYMTYPE '2'   /* reserved */
#define CHRTYPE '3'   /* character special */
#define BLKTYPE '4'   /* block special */
#define DIRTYPE '5'   /* directory */
#define FIFOTYPE '6'  /* FIFO special */
#define CONTTYPE '7'  /* reserved */

#define XHDTYPE 'x' /* Extended header referring to the \
                       next file in the archive */
#define XGLTYPE 'g' /* Global extended header */

/* Bits used in the mode field, values in octal.  */
#define TSUID 04000   /* set UID on execution */
#define TSGID 02000   /* set GID on execution */
#define TSVTX 01000   /* reserved */
                      /* file permissions */
#define TUREAD 00400  /* read by owner */
#define TUWRITE 00200 /* write by owner */
#define TUEXEC 00100  /* execute/search by owner */
#define TGREAD 00040  /* read by group */
#define TGWRITE 00020 /* write by group */
#define TGEXEC 00010  /* execute/search by group */
#define TOREAD 00004  /* read by other */
#define TOWRITE 00002 /* write by other */
#define TOEXEC 00001  /* execute/search by other */

/*
Instead create record logic within the functions and set the record so can
be adjusted

typedef struct tar_record{
    union block blocks[20];
}tar_record;
*/

// ??? typedef struct >>> for 2 zero blocks + the 18 of padding

void flag_error();
void file_error(char *file_name);
void failed_alloc();
void print_string_array(char **all_names, int num_names);
void free_string_array(char **names, int num_names);
header *fill_header_info(char * file);


char **create_names_array(int argc, char **argv, int num_names);

int create_file(char *file_name, int flags, int perms);
int create_tar(char **names, int num_names, int v_flag);
int append_file(int tar_fd, char *append_file);
int update_tar(int argc, char **argv);
int list_tar(int argc, char **argv, int v_flag);
int extract_tar(int argc, char **argv, int v_flag);

// for things like this will need a pointer to the beginning
// of the header and then can use pointer arithmetic to get
// where I need to go

// LIKELY NEED TO CALCULATE CHECKSUM... HERE IS AN EXAMPLE FROM DS
// / Function to calculate checksum
// unsigned int calculate_checksum(struct file_header *header) {
//     unsigned int sum = 0;
//     char *p = (char *)header;
//     for (int i = 0; i < RECORDSIZE; i++) {
//         sum += p[i];
//     }
//     return sum;
// }

// tar -czf -t >>> will throw error
// parse the files but if already c_flag, etc. is 1 then file_error(argv[i])
// if t then cannot have c, r, t, u, x ... but needs f
// -f to parse file correctly

int main(int argc, char **argv)
{
    // c,f,r,t,u,x            + v, z, j flags
    int c_flag = 0;
    int f_flag = 0;
    int r_flag = 0;
    int t_flag = 0;
    int u_flag = 0;
    int x_flag = 0;

    int v_flag = 0;
    int z_flag = 0;
    int j_flag = 0;
    int num_flag_args = 1; // for now is 1 to make it work

    int num_names = argc - num_flag_args + 1;
    char **names = create_names_array(argc, argv, num_names);
    if (!names)
    {
        failed_alloc();
        return 1;
    }

    if (argc == 1)
    {
        flag_error();
        return 1;
    }
    //*********************need to count the number of dashes and redo this section!!! tar -c -f  versus  tar -cf name.tar file_name
    else if (my_strcmp(argv[1], "-cf") == 0)
    {
        if(create_tar(names, num_names, v_flag) == -1)
        {
            return 1;
        }
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
        t_flag = 1;
        f_flag = 1;
    }
    // else if(my_strcmp(argv[1], "-tvf")==0)
    //     {
    //         t_flag = 1;
    //         v_flag = 1;
    //         f_flag = 1;
    //     }
    else if (my_strcmp(argv[1], "-uf") == 0)
    {
        u_flag = 1;
        f_flag = 1;
    }
    else if (my_strcmp(argv[1], "-xf") == 0)
    {
        x_flag = 1;
        f_flag = 1;
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
        return 1;
    }

    my_printf("argc: %d\n", argc);
    my_printf("c: %d, f: %d, j: %d, r: %d, t: %d, u: %d, v: %d, x: %d, z: %d\n", c_flag, f_flag, j_flag, r_flag, t_flag, u_flag, v_flag, x_flag, z_flag);

    //*** NEED TO IMPLEMENT num_flag_args ABOVE */

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

void tarball_error(char *tar_name)
{
    my_printf("my_tar: Cannot open %s\n", tar_name);
}

void print_string_array(char **all_names, int num_names)
{
    for (int i = 0; i < num_names; i++)
    {
        printf("%s\n", all_names[i]);
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

    print_string_array(names, names_index);
    return names;
}

int create_file(char *file_name, int flags, int perms)
{
    int fd;
    if (fd = open(file_name, O_RDWR, perms) == -1)
    {
        tarball_error(file_name);
        return 1;
    }
    return fd;
}


header *fill_header_info(char * file)
{
   header *file_header = malloc(sizeof(header *)); 
   if(!file_header)
   {
    //failed_alloc(); ???
    return NULL;
   }   
   my_memset(file_header, 0, sizeof(header)); // Zero out the memory
   my_strlen(file);

   my_strncpy(file_header->name,file, NAMESIZE-1); 
}


int append_file(int tar_fd, char *append_file)
{
    header *file_header = fill_header_info(append_file);
    if(!file_header)
    {
        failed_alloc(); // ???
        return 1;
    }
//after writing file_header info to file FREE file_header !!!
//includes  fill_header(names[i]);>>> MAKE SURE TO START APPENDING WRITE WHEN ZERO PADDING STARTS AND MAKE

    return 0; //if successful may need conditional logic
}







int create_tar(char **names, int num_names, int v_flag)
{
    char *tar_name = names[0];
    int tar_fd;
    
    //create tar file:
    if (tar_fd = create_file(tar_name, O_RDWR, TAR_PERMS) < 0)
    {
        tarball_error(tar_name);
        return -1;
    }

    unsigned int needed_space = 0;

    for (int i = 1; i < num_names; i++)
    {
        struct stat arg_stats;
        if(stat(names[i], &arg_stats) < 0)
        {
            file_error(names[i]);
        }
        
        else if(S_ISDIR(arg_stats.st_mode))
        {
            //recursively go through folder and append to file
            //append_directory() ???
        }
        else
        {
            //if file to append
            if(append_file(tar_fd, names[i])!= 0)
            {
                return -1;  //???
            }
        // if(v_flag) >>> to print the file that was added
            // {
            //     my_printf("%s\n", names[i]);
            // }
            
        }
    }
    return tar_fd;
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
