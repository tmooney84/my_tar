#ifndef FILE_HEADER_FNS_H
#define FILE_HEADER_FNS_H

#include <stddef.h>
#include <sys/stat.h>
#define BLOCKSIZE 512
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
    char content[BLOCKSIZE];
} block;

header *fill_header_info(char *file);
void fill_name(char *file, header *file_header);
void fill_mode(char *file, struct stat file_stats, header *file_header);
void fill_uid(char *file, struct stat file_stats, header *file_header);
void fill_gid(char *file, struct stat file_stats, header *file_header);
void fill_size(char *file, struct stat file_stats, header *file_header);
void fill_mtime(char *file, struct stat file_stats, header *file_header);
void fill_typeflag(char *file, struct stat file_stats, header *file_header);
void fill_linkname(char *file, struct stat file_stats, header *file_header);
void fill_uname(char *file, struct stat file_stats, header *file_header);
void fill_gname(char *file, struct stat file_stats, header *file_header);
void fill_devmajor(char *file, struct stat file_stats, header *file_header);
void fill_devminor(char *file, struct stat file_stats, header *file_header);
void fill_chksum(header *file_header);

void ld_to_string(long int number, char string[], int os_size);
void int_to_oct_string(int number, char octal_string[], int os_size);

//Tester:
int tester_main(char file_name []);

#endif
