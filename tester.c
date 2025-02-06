// #include "print_error.h"
// #include "my_printf.h"
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
// #include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"

#define RECORDSIZE 512
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32

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

header *fill_header_info(char *file);
void fill_name(char *file, header *file_header);
void fill_mode(char *file, struct stat file_stats, header *file_header);
void fill_uid(char *file, struct stat file_stats, header *file_header);
void fill_gid(char *file, struct stat file_stats, header *file_header);
void fill_size(char *file, struct stat file_stats, header *file_header);
void fill_mtime(char *file, struct stat file_stats, header *file_header);
void fill_typeflag(char *file, struct stat file_stats, header *file_header);

void ld_to_string(long int number, char string[], int os_size);
void int_to_oct_string(int number, char octal_string[], int os_size);

int main()
{
    header *test_header = fill_header_info("my_printf.c");
    if (!test_header)
    {
        printf("Error... try again");
        return 1;
    }

    printf("pid_t: %zu\n", sizeof(pid_t));
    printf("uid_t: %zu\n", sizeof(uid_t));
    printf("gid_t: %zu\n", sizeof(gid_t));

    printf("File Name: %s\n", test_header->name);
    printf("File Mode: %s\n", test_header->mode);
    printf("File UID: %s\n", test_header->uid);
    printf("File GID: %s\n", test_header->gid);
    printf("File Size: %s\n", test_header->size);
    printf("File mtime: %s\n", test_header->mtime);

    free(test_header);
    return 0;
}

header *fill_header_info(char *file)
{
    struct stat file_stats;
    //= malloc(sizeof(struct stat));
    // if (!file_stats)
    // {
    //     return NULL;
    // }

    header *file_header = malloc(sizeof(header));
    if (!file_header)
    {
        // free(file_stats);
        return NULL;
    }
    my_memset(file_header, 0, sizeof(header)); // Zero out the memory

    fill_name(file, file_header);
    fill_mode(file, file_stats, file_header);
    fill_uid(file, file_stats, file_header);
    fill_gid(file, file_stats, file_header);
    fill_size(file, file_stats, file_header);
    fill_mtime(file, file_stats, file_header);
    fill_typeflag(file, file_stats, file_header);

    return file_header;

    }

void int_to_oct_string(int number, char octal_string[], int os_size)
{
    for (int i = os_size - 2; i >= 0; i--)
    {
        octal_string[i] = '0' + (number % 8);
        number /= 8;
    }
    printf("octal_string: %s\n", octal_string);
}


void ld_to_string(long int number, char string[], int os_size)
{
    for (int i = os_size - 2; i >= 0; i--)
    {
        string[i] = '0' + (number % 10);
        number /= 10;
    }
    printf("dec_string: %s\n", string);
}


// char name[NAMESIZE]; /*   0 */
void fill_name(char *file, header *file_header)
{
    int file_size = (my_strlen(file) < NAMESIZE - 1) ? my_strlen(file) : NAMESIZE - 1;
    my_strncpy(file_header->name, file, file_size);
}

//     char mode[8];        /* 100 */
void fill_mode(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }

    // 07777 mask that covers S_IRWXU, S_IRWXG, S_IRWXO
    mode_t file_stats_mode = file_stats.st_mode & 07777;
    int result_mode = (int)file_stats_mode;

    char octal_string[8];
    my_memset(octal_string, 0, 8);

    octal_string[7] = '\0';

    int_to_oct_string(result_mode, octal_string, 8);
    my_strncpy(file_header->mode, octal_string, 8);
}

//     char uid[8];         /* 108 */
void fill_uid(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    int uid_int = file_stats.st_uid;

    char octal_string[8];
    my_memset(octal_string, 0, 8);
    octal_string[7] = '\0';

    int_to_oct_string(uid_int, octal_string, 8);
    my_strncpy(file_header->uid, octal_string, 8);
}


//     char gid[8];         /* 116 */
void fill_gid(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    int gid_int = file_stats.st_gid;
    printf("gid: %d\n", gid_int);
    char octal_string[8];
    my_memset(octal_string, 0, 8);
    octal_string[7] = '\0';

    int_to_oct_string(gid_int, octal_string, 8);
    my_strncpy(file_header->gid, octal_string, 8);
}


    //     char size[12];       /* 124 */
void fill_size(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    long int gid_ld = (long int)file_stats.st_size;
    char string[12];
    my_memset(string, 0, 12);
    string[11] = '\0';

    ld_to_string(gid_ld, string, 12);
    my_strncpy(file_header->size, string, 12);
}


    //     char mtime[12];      /* 136 */
void fill_mtime(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    long int mtime_ld = (long int)file_stats.st_mtime;
    char string[12];
    my_memset(string, 0, 12);
    string[11] = '\0';

    ld_to_string(mtime_ld, string, 12);
    my_strncpy(file_header->mtime, string, 12);
}

//     char chksum[8];      /* 148 */
//DO AT THE END!!!!!!!


    //     char typeflag;       /* 156 */
 void fill_typeflag(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    //S_IFMT is a macro for 0170000 Mask to filter out file bits
    mode_t file_stats_mode = file_stats.st_mode & __S_IFMT;
    int result_mode = (int)file_stats_mode;

    printf("File Type: %o\n", result_mode); 
    //char octal_string[8];
   // my_memset(octal_string, 0, 8);

    //octal_string[7] = '\0';

//    int_to_oct_string(result_mode, octal_string, 8);
    //my_strncpy(file_header->mode, octal_string, 8);
}   





    //     char linkname[100];  /* 157 */
    //     char magic[6];       /* 257 */
    //     char version[2];     /* 263 */
    //     char uname[TUNMLEN]; /* 265 */
    //     char gname[TGNMLEN]; /* 297 */
    //     char devmajor[8];    /* 329 */
    //     char devminor[8];    /* 337 */
    //     char prefix[155];    /* 345 */
    //                          /* 500 */
    //     char padding[12];
