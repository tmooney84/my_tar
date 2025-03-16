#ifndef MAIN_H
#define MAIN_H

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
#include "hashmap.h"

#define RECORDSIZE 20
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32
#define PREFIXSIZE 155
#define BLOCKSIZE 512
#define MAX_FILENAME 255

#define PATH_MAX 4096
// better to use a union? padding needed?

#define TAR_PERMS 0664 // RW for owner, group & R for others

#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2

void flag_error();
void file_error(char *file_name);
void failed_alloc();
void append_error();
void tarball_error(char *tar_name);
void file_not_found_error(char *file_name);
void previous_errors();
void print_string_array(char **all_names, int num_names);
void free_string_array(char **names, int num_names);
header *fill_header_info(char *file);
char **create_names_array(int argc, char **argv, int num_names);
int open_tar(char **names);
int open_file(char *file_name, int flags, int perms);
int print_included_tar_contents(int tar_fd, char **names, int num_names);
int create_tar(char **names, int num_names); // int v_flag
int create_tar_file(char *tar_name, char op_flag);
off_t append_file_data(int tar_fd, char *append_file);
int update_tar(int argc, char **argv);
int list_tar(int argc, char **argv, int v_flag);
int extract_tar(char **names, int num_names); // int v_flag
off_t add_zeros(int tar_fd);
int extract_all_contents(int tar_fd, char **names_to_extract, int num_ex_names);
int extract_process_entry(header *f_header, int tar_fd, int current_block);
off_t process_entry(char *path, int tar_fd);
int write_header(header *hdr, int tar_fd);
int write_file_data(int dst_fd, int src_fd, int f_size, int tar_flag);
off_t write_padding(int tar_fd, int total_required_padding);
int map_file_metadata(header *f_header, int fd);
int map_dir_metadata(header *f_header, char *file_name);
size_t parse_octal(char *str, size_t max_len);
char *parse_dir_slash(char *file_name);
int archive_tar(char *tar_name, char **archive_names, int num_archive_names, char op_flag); // int v_flag
#endif
