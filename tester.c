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

    return file_header;

    //     char uid[8];         /* 108 */
    //     char gid[8];         /* 116 */
    //     char size[12];       /* 124 */
    //     char mtime[12];      /* 136 */
    //     char chksum[8];      /* 148 */
    //     char typeflag;       /* 156 */
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
    int result = (int)file_stats_mode;

    printf("Dec_result_t: %o\n", result);
    printf("Test file_stats_mode: %o\n", file_stats_mode);
    char octal_string[8];
    my_memset(octal_string, 0, 8);

    octal_string[7] = '\0';

    int_to_oct_string(result, octal_string, 8);
    my_strncpy(file_header->mode, octal_string, 8);
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
