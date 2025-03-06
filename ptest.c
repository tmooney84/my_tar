#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <utime.h>
#include <time.h>
#include <sys/time.h>

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

void *my_memset(void *str, int c, size_t n)
{
    unsigned char *ptr = (unsigned char *)str;
    // loops over each byte
    for (size_t i = 0; i < n; i++)
    {
        /*
        -typecasting because the compiler only
        -may truncate c or leads to warnings of mismatched types
        -typecasting to unsigned char will guarentee that only the
        -least significant byte is used in relevent bitwise operations
        -allows portability across platforms
        */
        ptr[i] = (unsigned char)c;
    }

    return str;
}

int my_strlen(char *string)
{
    if (string == NULL)
    {
        return -1;
    }

    int i;
    for (i = 0; string[i] != '\0'; i++)
    {
        // loop to find the length
    }
    return i;
}

char * parse_dir_slash(char * file_name)
{
    int file_name_size = my_strlen(file_name);
    char * dir_name = malloc(sizeof(NAMESIZE));
    if(!dir_name)
    {
        printf("to create dir_name in parse_dir_slash()\n");
        return NULL;
    } 
    
    my_memset(dir_name, 0, NAMESIZE);
    int i = 0;
    for(; i < file_name_size; i++)
    {
        dir_name[i] = file_name[i];
    }
    
    if(file_name[file_name_size -1] == '/')
        {
        dir_name[file_name_size-1] = '\0';
        }
            
            return dir_name;
    }

    int main()
    {
        char name [NAMESIZE] = "dir1/dir2/";
        printf("Dir_name: %s", name);

        char *dir_name = parse_dir_slash(name);
        printf("after: %s", dir_name);

        free(dir_name);
        return 0;
    }