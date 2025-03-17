#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <linux/limits.h>

#include "utils.h"
#include "file_header_fns.h"
#include "print_error.h"
#include "my_printf.h"

#define BLOCKSIZE 512
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32
#define PREFIXSIZE 155

#define TMAGIC "ustar\0" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null ... "  " seems to be what regular tar uses...*/
#define TVERSLEN 2

int tester_main(char file_name[])
{
    header *file_header;

    file_header = fill_header_info(file_name);

    if (!file_header)
    {
        return 1;
    }

    my_printf("File Name: %s\n", file_header->name);
    my_printf("File Mode: %s\n", file_header->mode);
    my_printf("File UID: %s\n", file_header->uid);
    my_printf("File GID: %s\n", file_header->gid);
    my_printf("File Size: %s\n", file_header->size);
    my_printf("File mtime: %s\n", file_header->mtime);
    my_printf("File chksum: %s\n", file_header->chksum);
    my_printf("File typeflag: %c\n", file_header->typeflag);
    my_printf("File linkname: %s\n", file_header->linkname);
    my_printf("File magic: %s\n", file_header->magic);
    my_printf("File version: %.2s\n", file_header->version);
    my_printf("File uname: %s\n", file_header->uname);
    my_printf("File gname: %s\n", file_header->gname);
    my_printf("File devmajor: %s\n", file_header->devmajor);
    my_printf("File devminor: %s\n", file_header->devminor);
    my_printf("File prefix: %s\n", file_header->prefix);

    free(file_header);
    return 0;
}

header *fill_header_info(char *file)
{
    struct stat file_stats;

    header *file_header = malloc(sizeof(header));
    if (!file_header)
    {
        return NULL;
    }
    my_memset(file_header, '\0', sizeof(header)); // Zero out the memory

    fill_mode(file, file_stats, file_header);
    fill_uid(file, file_stats, file_header);
    fill_gid(file, file_stats, file_header);
    fill_mtime(file, file_stats, file_header);
    fill_typeflag(file, file_stats, file_header);
    fill_size(file, file_stats, file_header);
    fill_name(file, file_header);
    fill_linkname(file, file_stats, file_header);
    fill_uname(file, file_stats, file_header);
    fill_gname(file, file_stats, file_header);
    fill_devmajor(file, file_stats, file_header);
    fill_devminor(file, file_stats, file_header);
    fill_chksum(file_header);

    //      char magic[6];       /* 257 */
    file_header->magic[0] = 'u';
    file_header->magic[1] = 's';
    file_header->magic[2] = 't';
    file_header->magic[3] = 'a';
    file_header->magic[4] = 'r';
    file_header->magic[5] = ' '; //***/

    //     char version[2];     /* 263 */
    file_header->version[0] = ' ';  //***/
    file_header->version[1] = '\0'; //***/

    return file_header;
}

void int_to_oct_string(int number, char octal_string[], int os_size)
{
    for (int i = os_size - 2; i >= 0; i--)
    {
        octal_string[i] = '0' + (number % 8);
        number /= 8;
    }
}

void ld_to_oct_string(long int number, char string[], int os_size)
{
    for (int i = os_size - 2; i >= 0; i--)
    {
        string[i] = '0' + (number % 8);
        number /= 8;
    }
    string[os_size - 1] = '\0';
}

void ld_to_string(long int number, char string[], int os_size)
{
    for (int i = os_size - 2; i >= 0; i--)
    {
        string[i] = '0' + (number % 10);
        number /= 10;
    }
}

// char name[NAMESIZE]; /*   0 */
// char prefix[155];    /* 345 */
/* 500 */
int fill_name(char *file, header *file_header)
{
    char file_name[NAMESIZE];
    my_strncpy(file_name, file, NAMESIZE);
    int path_size = my_strlen(file);
    
    if(file_header->typeflag == '5' && path_size < NAMESIZE-2)
    {
        file_name[path_size] = '/';
        file_name[path_size + 1] = '\0';
    }

    path_size = my_strlen(file_name);
    int last_slash_point = -1;

    // due to my_strncpy placing a '\0' at the end of the string so
    // 99 characters can be the max
    if (path_size < NAMESIZE)
    {
        my_strncpy(file_header->name, file_name, path_size);
        file_header->name[path_size] = '\0';
        return 0;
    }
    if (path_size >= NAMESIZE)
    {
        for (int i = 0; i < path_size; i++)
        {
            if (file_name[i] == '/')
            {
                last_slash_point = i;
            }
        }
    }

    if (last_slash_point == -1 || last_slash_point >= PREFIXSIZE)
    {
        print_error("Relative Path too large for TAR format");
        return -1;
    }
    my_strncpy(file_header->prefix, file_name, last_slash_point);
    file_header->prefix[last_slash_point] = '\0';

    // Copy the filename portion into name (up to 100 bytes)
    my_strncpy(file_header->name, file_name + last_slash_point + 1, NAMESIZE - 1);
    file_header->name[NAMESIZE - 1] = '\0';

    return 0;
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
    file_header->uid[7] = '\0';
}

//     char gid[8];         /* 116 */
void fill_gid(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    int gid_int = file_stats.st_gid;
    char octal_string[8];
    my_memset(octal_string, 0, 8);
    octal_string[7] = '\0';

    int_to_oct_string(gid_int, octal_string, 8);
    my_strncpy(file_header->gid, octal_string, 8);
    file_header->gid[7] = '\0';
}

//     char size[12];       /* 124 */
void fill_size(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }
    long int f_size;
    
    if(file_header->typeflag == '5')
    {
        f_size = 0;
    }
    else
    {
    f_size = (long int)file_stats.st_size;
    }
    char string[12];
    my_memset(string, 0, 12);
    string[11] = '\0';

    ld_to_oct_string(f_size, string, 12);
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

    ld_to_oct_string(mtime_ld, string, 12);
    my_strncpy(file_header->mtime, string, 12);
}

void fill_typeflag(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return; // If stat fails, return without modifying typeflag
    }

    // Use the actual values instead of macros
    switch (file_stats.st_mode & 0170000) // 0170000 is the bitmask for file type
    {
    case 0100000: // Regular file
        file_header->typeflag = '0';
        break;
    case 0120000: // Symbolic link
        file_header->typeflag = '2';
        break;
    case 0020000: // Character device
        file_header->typeflag = '3';
        break;
    case 0060000: // Block device
        file_header->typeflag = '4';
        break;
    case 0040000: // Directory
        file_header->typeflag = '5';
        break;
    case 0010000: // FIFO (named pipe)
        file_header->typeflag = '6';
        break;
    default: // Unknown or unsupported file type
        file_header->typeflag = '\0';
        break;
    }
}

//     char linkname[100];  /* 157 */
void fill_linkname(char *file, struct stat file_stats, header *file_header)
{
    if (lstat(file, &file_stats) == -1)
    {
        return;
    }

    //if (S_ISLNK(file_stats.st_mode))
    if ((file_stats.st_mode & 0170000) == 0120000) 
    {
        ssize_t file_len = readlink(file, file_header->linkname, sizeof(file_header->linkname) - 1);
        if (file_len == -1)
        {
            return;
        }
        file_header->linkname[file_len] = '\0';
    }
    else
    {
        file_header->linkname[0] = '\0';
    }
}

//     char uname[TUNMLEN]; /* 265 */
void fill_uname(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }

    struct passwd *pwd = getpwuid(file_stats.st_uid);
    if (pwd)
    {
        my_strncpy(file_header->uname, pwd->pw_name, TUNMLEN - 1);
    }
    else
    {
        my_strncpy(file_header->uname, "unknown", TUNMLEN - 1);
    }

    file_header->uname[TUNMLEN - 1] = '\0';
}

//     char gname[TGNMLEN]; /* 297 */
void fill_gname(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }

    struct group *grp = getgrgid(file_stats.st_gid);
    if (grp)
    {
        my_strncpy(file_header->gname, grp->gr_name, TGNMLEN - 1);
    }
    else
    {
        my_strncpy(file_header->gname, "unknown", TGNMLEN - 1);
    }

    file_header->gname[TGNMLEN - 1] = '\0';
}

//     char devmajor[8];    /* 329 */
void fill_devmajor(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }

    //if (S_ISBLK(file_stats.st_mode) || S_ISCHR(file_stats.st_mode))
    if ((file_stats.st_mode & 0170000) == 0060000 || // Block device
        (file_stats.st_mode & 0170000) == 0020000)   // Character device
    {
        //dev_t file_devmajor = major(file_stats.st_dev);
        dev_t file_devmajor = (file_stats.st_dev >> 8) & 0xFF; // Assuming major is in the upper 8 bits
        
        int_to_oct_string(file_devmajor, file_header->devmajor, 8);
    }
}

//     char devminor[8];    /* 337 */
void fill_devminor(char *file, struct stat file_stats, header *file_header)
{
    if (stat(file, &file_stats) == -1)
    {
        return;
    }

    //if (S_ISBLK(file_stats.st_mode) || S_ISCHR(file_stats.st_mode))
    if ((file_stats.st_mode & 0170000) == 0060000 || // Block device
        (file_stats.st_mode & 0170000) == 0020000)   // Character device
    {
        //dev_t file_devminor = minor(file_stats.st_dev);
        dev_t file_devminor = file_stats.st_dev & 0xFF; // Assuming minor is in the lower 8 bits
        int_to_oct_string(file_devminor, file_header->devminor, 8);
    }
}

void fill_chksum(header *file_header)
{
    //'0's to fill out checksum field
    my_memset(file_header->chksum, ' ', 8);
    int chksum_total = 0;

    unsigned char *header_bytes = (unsigned char *)file_header;
    // loop over the entire header adding to chksum_total
    // casts to unsigned char (bytes) datatype for looping
    for (int i = 0; i < BLOCKSIZE; i++)
    {
        // not counting chksum portion of header
        if (i >= 148 && i < 156)
        {
            chksum_total += ' ';
        }
        else
        {
            chksum_total += header_bytes[i];
        }
    }

    int_to_oct_string(chksum_total, file_header->chksum, 7);
    file_header->chksum[6] = '\0';
    file_header->chksum[7] = ' ';
}