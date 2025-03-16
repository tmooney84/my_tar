#ifndef HASHMAP_H
#define HASHMAP_H

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

#define BLOCKSIZE 512
#define NAMESIZE 100
#define MAX_FILENAME 255

typedef struct file_entry
{
    int key;
    char name[NAMESIZE - 1];
    int newest_version_flag;
    int file_exists_flag;
    time_t mod_time;
    struct file_entry *next;
} File_Entry;

typedef struct Hashtable
{
    File_Entry **buckets;
    int num_buckets;
    int num_vetted_names;
} Hashtable;

typedef struct names_list
{
    char **names;
    int num_names;
} Names_List;


Hashtable *get_update_names(char **input_names, int input_num_names);
Hashtable *create_table(int num_buckets);
void free_names_list(Names_List *list);
int add_entry(File_Entry *entry, Hashtable *table);
void free_table(Hashtable *table);
int hash_fn(char *name, int num_buckets);
Hashtable *build_prompt_names_table(char **names, int num_names);
int check_newest_names(int tar_fd, Hashtable *table);
Names_List *get_newest_names(Hashtable *table);
int get_mod_times(Hashtable *table);
int check_files_exist(Hashtable *table);
int print_error_names(Hashtable *table);
void print_table(Hashtable *table);
#endif