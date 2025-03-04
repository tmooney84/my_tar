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

#include "utils.h"
#include "my_printf.h"
#include "print_error.h"
#include "file_header_fns.h"

#define TAR_PERMS 0664 // RW for owner, group & R for others

int map_file_metadata(header *f_header, int fd)
{
    struct stat file_stats;
    if (fstat(fd, &file_stats) == -1)
    {
        return -1;
    }

    //** need to make sure mode has the compressed permissions and file type */
    switch (f_header->typeflag)
    {
    case '0':
        file_stats.st_mode = (mode_t)0100000 & parse_octal("0000644", sizeof(f_header->mode));
        break;

    case '2':
        file_stats.st_mode = (mode_t)0120000 & parse_octal(f_header->mode, sizeof(f_header->mode));
        break;

    default:
        // file_stats.st_mode = '\0';
        print_error("Unable to set mode\n");
        break;
    }

    ///...if others needed get from fill_typeflag

    file_stats.st_uid = (unsigned int)parse_octal("123", sizeof(f_header->uid));
    file_stats.st_gid = (unsigned int)parse_octal("123", sizeof(f_header->gid));
    file_stats.st_size = lseek(fd, 0, SEEK_END); // could do parse_octal(f_header->size, 12);
    file_stats.st_mtime = parse_octal("14760465573", sizeof(f_header->mtime));
   
    printf("----------------------------------------------------------\n");
    printf("MODIFIED TIME: %lu\n", (long unsigned int)file_stats.st_mtime);
    //file_stats.st_mtime = parse_octal(f_header->mtime, sizeof(f_header->mtime));
    //file_stats.st_mtime = (__time_t) parse_octal(f_header->mtime, sizeof(f_header->mtime));

    // need to look for fill_devmajor and fill_devminor if need to fill st_dev and st_rdev for block and char device

    /* REMEMBER OCT STRING TO INT
   st_mode    chmod()   → Derived from the tar header’s mode and typeflag
st_uid and st_gid    chown()   → Derived from the header’s uid and gid
st_size → Tells you how many bytes of file data to read
st_mtime    utime()    futime()   → Derived from the header’s mtime

map_file_data(f_header, fd)
*/
    return 0;
}

int create_file(char *file_name, int flags, int perms)
{
    int fd;
    fd = open(file_name, flags, perms);
    if (fd < 0)
    {
        tarball_error(file_name);
        return -1;
    }
    return fd;
}

int main()
{
    char file_name[100] = "test123456.txt";
    int fd = create_file(file_name, O_RDWR | O_CREAT | O_TRUNC, TAR_PERMS);
}