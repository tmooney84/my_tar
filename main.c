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

Exit Status >>> return 0 on success >0 if an error occurs (return -1)

Errors handling Errors will be written on STDERR.
File not found (provided file is: i_don_t_exist): my_tar: i_don_t_exist: Cannot stat: No such file or directory

Error with the tarball file (provided file is: tarball.tar): my_tar: Cannot open tarball.tar
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"
#include "my_printf.h"
#include "print_error.h"

#define RECORDSIZE 512
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32

//better to use a union? padding needed?
union block {

char content[512];

struct file_header
{                              /* byte offset */
  char name[NAMESIZE];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[TUNMLEN];               /* 265 */
  char gname[TGNMLEN];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
  char padding[12];             //POTENTIAL PADDING
}file_header;
} block;

typedef struct tar_record{
    union block blocks[20];
} tar_record;

// ??? typedef struct >>> for 2 zero blocks + the 18 of padding

void flag_error();
void file_error(char * file_name);
void failed_alloc();
void print_string_array(char **all_names, int num_names);
void free_string_array(char **names, int num_names);
int create_tar(int argc, char ** argv, int v_flag);
int archive_tar(int argc, char **argv);
int update_tar(int argc, char **argv);
int list_tar(int argc, char **argv, int v_flag);
int extract_tar(int argc, char **argv, int v_flag);


//for things like this will need a pointer to the beginning
//of the header and then can use pointer arithmetic to get
//where I need to go

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

    if (argc == 1)
    {
        flag_error();
        return -1;
    }

else if(my_strcmp(argv[1], "-cf")==0)
    {
        create_tar(argc, argv, v_flag);
    }
// else if(my_strcmp(argv[1], "-cvf")==0)
//     {
//         v_flag = 1;
//         create_tar(argc, argv, v_flag);
//     }
else if(my_strcmp(argv[1], "-rf")==0)
    {
        archive_tar(argc, argv);
    }
else if(my_strcmp(argv[1], "-tf")==0)
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
else if(my_strcmp(argv[1], "-uf")==0)
    {
        u_flag = 1;
        f_flag = 1;
    }   
else if(my_strcmp(argv[1], "-xf")==0)
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
    return -1;
}

    my_printf("argc: %d\n", argc);
    my_printf("c: %d, f: %d, j: %d, r: %d, t: %d, u: %d, v: %d, x: %d, z: %d\n", c_flag, f_flag, j_flag, r_flag, t_flag, u_flag, v_flag, x_flag, z_flag);
    
    char ** names = malloc ((argc -2) * sizeof(char *));
    if(!names)
    {  
        failed_alloc(); 
        return -1;
    }
    my_memset(names, 0, (argc - 2) * (sizeof(char *)));

    int names_index = 0;

    for(int i = 2; i < argc; i++)
    {   
        int names_len = my_strlen(argv[i]);
        names[names_index] = malloc((my_strlen(argv[i]) + 1) * sizeof(char));  
        if(!names[names_index])
        {
            failed_alloc();
            return -1;
        } 
        my_memset(names[names_index], 0, names_len);

        my_strncpy(names[names_index], argv[i], names_len);
        names_index++;
    }

    print_string_array(names, names_index);


//argv[2]
//test whether string file or folder if not error

//-cf -rf -uf      create archive? add to archive? add to archive if update (stat time conditional)? >>> 
//this could all be one fn w flags

//print out archive contents >>> additional function

//extract archive
    return 0;
}

void flag_error()
{
    my_printf("tar: You must specify one of the '-Acdtrux', '--delete' or '--test-label' options\nTry 'tar --help' or 'tar --usage' for more information.");
}


void file_error(char * file_name)
{
    print_error("my_tar: %s: Cannot stat: No such file or directory", file_name); 
}

void failed_alloc()
{       
    my_printf("Failed to allocate memory.\n");
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
        free(names[i]);  // Free each dynamically allocated string
    }

    free(names);
} 


 create_tar(int argc, char ** argv, int v_flag)
{
//fill out file_header with appropriate info
// ^^^ this may be its own sub-function?

//determine the file size and the number of records + zero block records to be allocated

//copy the file in byte by byte into each block until filled, if not exact fit into last block of 
//record >>> add padding
//^^^ this may be a sub-function

//verbose printing of file completed need to test -v

//add zero blocks record to end of file
//^^^ this may be a sub-function

//verbose printing of tar completed >>> need to test -v

//return 0 success -1 for failure
}


int archive_tar(int argc, char **argv)
{

}


int update_tar(int argc, char **argv)
{

}


int list_tar(int argc, char **argv, int v_flag)
{

}


int extract_tar(int argc, char **argv, int v_flag)
{

}