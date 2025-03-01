#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <utime.h>
#include <string.h>

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

// int extract_all_contents(int tar_fd, char **names_to_extract, int num_ex_names)
//{
//  struct stat tar_stats;
//  if (fstat(tar_fd, &tar_stats) == -1)
//  {
//      print_error("Unable to stat tar\n");
//      return -1;
//  }

// long int tar_size = (long int)tar_stats.st_size;
// int total_blocks = tar_size / BLOCKSIZE;
// if (tar_size / BLOCKSIZE != 0)
// {
//     print_error("Error non-uniform tar size\n");
//     return -1;
// }

// int current_block = 0;

// // makes sure tar_fd is at beginning of the file
// lseek(tar_fd, 0, SEEK_SET);

// unsigned char header_block[512];
// my_memset(header_block, 0, sizeof(header_block));

// {
//     while (current_block < total_blocks)
//     {
//         int n = 0;
//         int returned_blocks = 0;

//         // set block to current location ???
//         // lseek(tar_fd, current_block * 512, SEEK_SET);

//         if ((n = read(tar_fd, header_block, 512) < 0) && n != 512)
//         {
//             print_error("Unable to read magic tar file\n");
//             return -1;
//         }
//         current_block++;

//         struct header *f_header = (struct header *)header_block;

//         // Extracting the entire tar file
//         if (my_strcmp(f_header->magic, "ustar") == 0 && (names_to_extract == NULL))
//         {
//             // do I need written_blocks?
//             if ((returned_blocks = extract_process_entry(f_header, tar_fd, current_block)) < 0)
//             {
//                 print_error("Error... unable to extract file from tar\n");
//                 return -1;
//             }
//             current_block = returned_blocks;
//         }

// Extracting specific file names

// if (my_strcmp(f_header->magic, "ustar") == 0 && (names_to_extract != NULL))
//{
//  {

// int extract_all_contents(int tar_fd, char **names_to_extract, int num_ex_names)

int test_names(char **names_to_extract, int num_ex_names, char **header_names, int num_header)
{
    printf("header_string2: %s\n", header_names[2]);
    printf("names2: %s\n", names_to_extract[2]);
    
    for (int m = 0; m < num_header; m++)
    {
        for (int i = 0; i < num_ex_names; i++)
        {
            if ((names_to_extract[i] != 0) && strcmp(header_names[m], names_to_extract[i]) == 0)
            {
                // if ((returned_blocks = extract_process_entry(f_header, tar_fd, current_block)) < 0)
                // {
                //     print_error("Error... unable to extract file from tar\n");
                //     return -1;
                // }
                names_to_extract[i] = 0;

               // current_block = returned_blocks;
            }
        }
    }

    // prints errors for those file names not found
    int error_flag = 0;
    for (int i = 0; i < num_ex_names; i++)
    {
        if (names_to_extract[i] == 0)
        {
            continue;
        }

        printf("Name not found in tar: %s\n", names_to_extract[i]);
        error_flag = 1;
    }
    if (error_flag == 1)
    {
        printf("previous errors\n");
    }
    else
    {
        printf("no errors\n");
    }

    return 0;
}

int main()
{
    char **names;
    names = malloc(5 * sizeof(char *));
    for (int i = 0; i < 5; i++)
    {
        names[i] = malloc(100 * sizeof(char));
    }

    strcpy(names[0], "file0.txt");
    strcpy(names[1], "file1.txt");
    strcpy(names[2], "file2.txt");
    strcpy(names[3], "file3.txt");
    strcpy(names[4], "baluga.txt");


printf("names 2: %s\n", names[2]);

    char **header_names;
    header_names = malloc(5 * sizeof(char *));
    for (int i = 0; i < 5; i++)
    {
        header_names[i] = malloc(100 * sizeof(char));
    }


    strcpy(header_names[0], "file0.txt");
    strcpy(header_names[1], "file1.txt");
    strcpy(header_names[2], "file2.txt");
    strcpy(header_names[3], "file3.txt");
    strcpy(header_names[4], "file4.txt");

printf("header_names 2: %s\n", header_names[2]);

    if(test_names(names, 5, header_names, 5) < 0)
    {
        printf("issues!!!\n");
        return -1;
    }


    for (int i = 0; i < 5; i++)
    {
        free(names[i]);
        free(header_names[i]);
    }

    free(names);
    free(header_names);

    return 0;
}
