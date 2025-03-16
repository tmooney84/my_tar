#include <string.h>

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
#include "tar_utils.h"
#include "my_printf.h"
#include "print_error.h"
#include "file_header_fns.h"
#include "hashmap.h"

#define NAMESIZE 100
#define PREFIXSIZE 155
#define MAX_FILENAME 255
#define TAR_PERMS 0664 // RW for owner, group & R for others

Hashtable *create_table(int num_buckets)
{
    Hashtable *table = malloc(sizeof(Hashtable));
    if(table == NULL)
    {
        failed_alloc();
        return NULL;
    }
    table->num_buckets = num_buckets;
    table->buckets = calloc(num_buckets, sizeof(File_Entry *));
    if(table->buckets == NULL)
    {
        free(table);
        failed_alloc();
        return NULL;
    }

    return table;
}

void free_names_list(Names_List *list)
{
    if (list == NULL)
    {
        return;
    }
    for (int i = 0; i < list->num_names; i++)
    {
        free(list->names[i]);
    }
    free(list->names);
}

int add_entry(File_Entry *entry, Hashtable *table)
{
    int duplicate_flag = 0;
    // IF ENTRY IS A DUPLICATE, FREE THE ENTRY???

    if (table->buckets[entry->key] == NULL)
    {
        table->buckets[entry->key] = entry;
    }

    else if (table->buckets[entry->key] != NULL)
    {
        File_Entry *head = table->buckets[entry->key];
        File_Entry *iterator = head;

        while (iterator != NULL)
        {
            if (my_strcmp(iterator->name, entry->name) == 0)
            {
                //!!! just for testing
                my_printf("entry: %s is a duplicate.\n", entry->name);
                duplicate_flag = 1;
                break;
            }
            iterator = iterator->next;
        }

        if (duplicate_flag == 0)
        {
            iterator->next = entry;
        }
    }

    else
    {
        // print_error("Unable to add entry to names hash table");
        print_error("Unable to add entry to names hash table");
        return -1;
    }

    return duplicate_flag;
}

// free table data
void free_table(Hashtable *table)
{
    if (table == NULL)
    {
        print_error("Table does not exist.");
        return;
    }
    for (int i = 0; i < table->num_buckets; i++)
    {
        File_Entry *head = table->buckets[i];
        File_Entry *tmp;

        while (head != NULL)
        {
            tmp = head;
            head = head->next;
            free(tmp);
        }
    }

    free(table->buckets);

    free(table);
}

int hash_fn(char *name, int num_buckets)
{
    int sum = 0;

    int name_len = my_strlen(name);

    for (int i = 0; i < name_len; i++)
    {
        sum += (int)name[i];
    }
    return sum % num_buckets;
}

Hashtable *build_prompt_names_table(char **names, int num_names)
{
    int num_buckets = 10; // for hashing_fn()
    Hashtable *names_table = create_table(num_buckets);
    if(names_table == NULL)
    {
        return NULL;
    }
    names_table->num_vetted_names = num_names;

    for (int i = 0; i < num_names; i++)
    {
        File_Entry *entry = malloc(sizeof(File_Entry));
        if (entry == NULL)
        {
            failed_alloc();
            free_table(names_table); 
            return NULL;
        }
        entry->key = hash_fn(names[i], num_buckets);
        my_strncpy(entry->name, names[i], NAMESIZE - 1); // i-1 to account for tar_name
        entry->name[NAMESIZE - 2] = '\0'; 
        entry->newest_version_flag = 1;
        entry->file_exists_flag = 0;
        entry->next = NULL;
        entry->mod_time = 0;

        int duplicate_flag = add_entry(entry, names_table);
        if (duplicate_flag < 0)
        {
            print_error("Unable to vet entry to hash table");
            free_table(names_table); 
            return NULL;
        }
        else if (duplicate_flag == 1)
        {
            --names_table->num_vetted_names;
            free(entry);
        }
    }
    return names_table;
}

/***********************Search functionality***********************/

int check_newest_names(int tar_fd, Hashtable *table)
{
    if (!table || !table->buckets || tar_fd < 0)
    {
        print_error("Invalid arguments\n in check_newest_names");
        return -1;
    }

    int num_vetted_names = table->num_vetted_names;
    //int num_newest_names = num_vetted_names;
    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        print_error("Unable to stat tar\n");
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;

    // make sure at beginning of tar_fd
    if (lseek(tar_fd, 0, SEEK_SET) < 0)
    {
        print_error("Unable to lseek file\n");
        return -1;
    }

    unsigned char header_buffer[512];
    int read_size = 0;

    while (read_size < tar_size)
    {
        my_memset(header_buffer, 0, sizeof(header_buffer));
        int n = 0;

        n = read(tar_fd, header_buffer, 512);
        if ((n < 0) || n != 512)
        {
            print_error("Unable to read magic tar file\n");
            return -1;
        }
        read_size += n;

        struct header *f_header = (struct header *)header_buffer;

        // Extracting the entire tar file
        //>>> have || on magic[5] due to strange things with tar in main.c... may need to look at
        if (f_header->magic[0] != 'u' ||
            f_header->magic[1] != 's' ||
            f_header->magic[2] != 't' ||
            f_header->magic[3] != 'a' ||
            f_header->magic[4] != 'r' ||
            (f_header->magic[5] != ' ' && f_header->magic[5] == '\0'))
        {
            continue;
        }

        char filename[NAMESIZE] = {0};
        my_strncpy(filename, f_header->name, NAMESIZE - 1);
        int last_slash_index = -1;

        for(int i = 0; filename[i] != '\0' && i < NAMESIZE; i++)
        {
            if(filename[i] == '/')
                {
                    last_slash_index = i;
                }
        }

        if(last_slash_index != -1)
        {
            filename[last_slash_index] = '\0';
        }

        int f_hash = hash_fn(filename, table->num_buckets);

        if (num_vetted_names == 0)
        {
            print_error("my_tar command needs additional arguments to add files to tar file.");
            return -1;
        }

        if (table->buckets[f_hash] == NULL)
        {
            print_error("No entry for bucket: %s\n", filename);
            return -1;
        }

        File_Entry *iterator = table->buckets[f_hash];

        while (iterator != NULL)
        {
            if ((my_strcmp(iterator->name, filename) == 0) && iterator->newest_version_flag == 1)
            {
                time_t f_mtime = (time_t)parse_octal(f_header->mtime, sizeof(f_header->mtime));

                // Debug prints (recommended)
                // my_printf("Checking %s: stored_mtime=%ld, tar_mtime=%ld\n",
                //        filename, iterator->mod_time, f_mtime);

                if (iterator->mod_time <= f_mtime)
                {
                    iterator->newest_version_flag = 0;
                    //num_newest_names--;
                }
                break;
            }
            iterator = iterator->next;
        }
    }
    return 0;
}

Names_List *get_newest_names(Hashtable *table)
{
    // First pass: Count valid entries
    int n = 0;
    int num_newest_names = 0;
    for (int i = 0; i < table->num_buckets && n < table->num_vetted_names; i++)
    {
        File_Entry *entry = table->buckets[i];
        while (entry != NULL)
        {
            if (entry->file_exists_flag && entry->newest_version_flag)
            {
                num_newest_names++;
            }
            n++;
            entry = entry->next;
        }
    }

    if (num_newest_names == 0)
    {
        return NULL;
    }

    // Allocate needed space for newest_names
    Names_List *newest_names = malloc(sizeof(Names_List));
    if (newest_names == NULL)
    {
        failed_alloc();
        return NULL;
    }

    newest_names->num_names = num_newest_names;

    newest_names->names = malloc(num_newest_names * sizeof(char *));
    if (newest_names->names == NULL)
    {
        failed_alloc();
        return NULL;
    }

    // Second pass: Copy names
    int index = 0;
    for (int i = 0; i < table->num_buckets && index < num_newest_names; i++)
    {
        File_Entry *entry = table->buckets[i];
        while (entry != NULL)
        {
            if (entry->file_exists_flag && entry->newest_version_flag)
            {
                newest_names->names[index] = malloc(NAMESIZE * sizeof(char));
                if (newest_names->names[index] == NULL)
                {
                    // Cleanup and return NULL
                    for (int j = 0; j < index; j++)
                    {
                        free(newest_names->names[j]);
                    }
                    free(newest_names);
                    return NULL;
                }
            my_strncpy(newest_names->names[index], entry->name, NAMESIZE - 1);
            index++;
            }
            entry = entry->next;
        }
    }
    return newest_names;
}

int get_mod_times(Hashtable *table)
{
    if (table == NULL)
    {
        print_error("Hashtable does not exist\n");
        return -1;
    }

    int n = 0;
    for (int i = 0; i < table->num_buckets && n < table->num_vetted_names; i++)
    {
        if (table->buckets[i] != NULL)
        {
            File_Entry *iterator = table->buckets[i];

            while (iterator != NULL)
            {
                if (iterator->file_exists_flag == 1)
                {
                    struct stat file_stats;
                    if (stat(iterator->name, &file_stats) == -1)
                    {
                        //*** file_error(table->buckets[i]->name);
                        return -1;
                    }
                    iterator->mod_time = file_stats.st_mtime;
                }
                n++;
                iterator = iterator->next;
            }
        }
    }
    return 0;
}

int check_files_exist(Hashtable *table)
{
    if (table == NULL)
    {
        print_error("Names hashtable does not exist\n");
        return -1;
    }

    int num_names = table->num_vetted_names;
    int num_buckets = table->num_buckets;

    DIR *dir;
    struct dirent *entry;

    if (num_names <= 0)
    {
        print_error("No names found in names_table.\n");
        return -1;
    }

    // checks if file paths exist relative to working directory
    if ((dir = opendir(".")) == NULL)
    {
        //!!! print_error("Error opening directory");
        perror("Error opening directory");
        return -1;
    }

    int n = 0;

    // traverse through each "potential file name" in the hash map and see if each exists in current directory
    for (int i = 0; i < num_buckets && n < num_names; i++)
    {
        if (table->buckets[i] != NULL)
        {
            File_Entry *iterator = table->buckets[i];

            while (iterator != NULL)
            {
                if (iterator->file_exists_flag == 0)
                {
                    rewinddir(dir);
                    while ((entry = readdir(dir)) != NULL)
                    {
                        if (my_strcmp(iterator->name, entry->d_name) == 0)
                        {
                            iterator->file_exists_flag = 1;
                            n++;
                            break;
                        }
                    }
                }

                iterator = iterator->next;
            }
        }
    }
    closedir(dir);
    return 0;
}

int print_error_names(Hashtable *table)
{
    // look at  ln 880 in main.c to see loop of file_not_found_error
    // and previous_errors(); are used and replicate with iteration
    int n = 0;
    int missing_names_flag = 0;

    for (int i = 0; i < table->num_buckets && n < table->num_vetted_names; i++)
    {
        if (table->buckets[i] != NULL)
        {
            File_Entry *iterator = table->buckets[i];

            while (iterator != NULL)
            {
                {
                    if (iterator->file_exists_flag == 0)
                    {
                        file_not_found_error(iterator->name);
                        missing_names_flag = 1;
                    }
                    n++;
                }
                iterator = iterator->next;
            }
        }
    }
    if (missing_names_flag)
    {
        previous_errors();
    }
    return 0;
}

void print_table(Hashtable *table)
    {
    my_printf("num_buckets: %d\nnum_vetted_names: %d\n", table->num_buckets, table->num_vetted_names);
    my_printf("\n");

    int n = 0;


    for (int i = 0; i < table->num_buckets && n < table->num_vetted_names; i++)
    {
        if (table->buckets[i] != NULL)
        {
            File_Entry *iterator = table->buckets[i];

            while (iterator != NULL)
            {
                {
                    my_printf("bucket[%d] key: %d\n", i, table->buckets[i]->key);
                    my_printf("bucket[%d] name: %s\n", i, table->buckets[i]->name);
                    my_printf("bucket[%d] file_exists_flag: %d\n", i, table->buckets[i]->file_exists_flag);
                    my_printf("bucket[%d] newest_version_flag: %d\n", i, table->buckets[i]->newest_version_flag);
                    my_printf("bucket[%d] mod_time: %lld\n", i, (long long int)table->buckets[i]->mod_time);
                    my_printf("\n");
                    n++;
                }
                iterator = iterator->next;
            }
        }
    }
    }

Hashtable *get_update_names(char **input_names, int input_num_names)
{
    int tar_fd;

    char *tar_name = input_names[0];
    if(tar_name == NULL)
    {
        print_error("Error with get_update_names input parameters.");
        return NULL;
    }
   
    tar_fd = create_tar_file(tar_name, 'u');
    if (tar_fd < 0)
    {
        tarball_error(tar_name);
        return NULL;
    }
    char **names = input_names + 1;
    int num_names = input_num_names - 1;

    //*****************PRINT OUT TEST************************/
    // my_printf("Inputted string names: \n");

    // for (int i = 0; i < num_names; i++)
    // {
    //     my_printf("names[%d]: %s\n", i, names[i]);
    // }
    /************************************************************************/

    Hashtable *table = build_prompt_names_table(names, num_names);
    if(table == NULL)
    {
        failed_alloc();
        return NULL;
    }
   
//    /***********************PRINTING FOR TESTING************ */ 
//     my_printf("Table pre-processing:\n"); 
//     my_print_table(table);
//     my_printf("\n");
//     /****************************************************** */


    if (check_files_exist(table) < 0)
    {
        print_error("Error checking whether files indicated in hashtable exist.\n");
        free_table(table); 
        return NULL;
    }

//  /***********************PRINTING FOR TESTING************ */ 
//     my_printf("Table post-check_files_exist():\n"); 
//     print_table(table);
//     my_printf("\n");
//     /****************************************************** */




    if (get_mod_times(table) < 0)
    {
        print_error("Error finding modification times for prompt file names.\n");
        free_table(table); 
        return NULL;
    }

//  /***********************PRINTING FOR TESTING************ */ 
//     printf("Table post-get_mod_times():\n"); 
//     print_table(table);
//     printf("\n");
//     /****************************************************** */


    if (check_newest_names(tar_fd, table) < 0)
    {
        print_error("Unable to check for newest names.\n");
        free_table(table);
        return NULL;
    }
 
    names = NULL;

    return table;
}