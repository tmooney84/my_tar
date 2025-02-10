#ifndef FILE_HEADER_FNS.H
#define FILE_HEADER_FNS.H

#include <stddef.h>

typedef struct header;

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
void fill_chksum(char *file, struct stat file_stats, header *file_header);

void ld_to_string(long int number, char string[], int os_size);
void int_to_oct_string(int number, char octal_string[], int os_size);

#endif
