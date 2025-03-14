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

// #include "utils.h"
// #include "my_printf.h"
// #include "print_error.h"
// #include "file_header_fns.h"

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
    size_t num_buckets;
    int num_prompt_names;
} Hashtable;

//     int append_tar(char **names, int num_names, char op_flag) // int v_flag
//  {
//      int tar_fd;

//      char *tar_name = names[0];
//      // printf("tar_name: %s\n", tar_name);
//      tar_fd = create_tar_file(tar_name, op_flag);
//      if (tar_fd < 0)
//      {
//          return -1;
//      }

//     int prev_error_flag = 0;

Hashtable *create_table(size_t num_buckets)
{
    Hashtable *table = malloc(sizeof(Hashtable));
    table->num_buckets = num_buckets;
    table->buckets = calloc(num_buckets, sizeof(File_Entry *));
    return table;
}

// add + collision linked list logic

// build_entry() fn needed? >>> one per file/dir name

int add_entry(File_Entry *entry, Hashtable *table)
{
    // unsigned char *hdr_data = (unsigned char *)hdr;

    // File_Entry *table_data = (File_Entry *)table;

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
            iterator = iterator->next;
        }
        iterator->next = entry;
    }

    else
    {
        // print_error("Unable to add entry to names hash table");
        printf("Unable to add entry to names hash table");
        return -1;
    }

    return 0;
}

// free table data
int free_table(Hashtable *table)
{
    if (table == NULL)
    {
        // print_error("Table does not exist.");
        printf("Table does not exist.");
        return -1;
    }
    for (size_t i = 0; i < table->num_buckets; i++)
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

    return 0;
}

int hash_fn(char *name, int num_buckets)
{
    int sum = 0;

    //***for (int i = 0; i < my_strlen(name); i++)
    for (int i = 0; i < strlen(name); i++)
    {
        sum += (int)name[i];
    }
    return sum % num_buckets;
}

Hashtable *build_prompt_names_table(char **names, int num_names)
{
    char **prompt_names = names + 1;
    int num_prompt_names = num_names - 1;
    int num_buckets = 10; // for hashing_fn()
    Hashtable *names_table = create_table(num_buckets);
    names_table->num_prompt_names = num_prompt_names;
    for (int i = 0; i < num_prompt_names; i++)
    {
        File_Entry *entry = malloc(sizeof(File_Entry));
        if (entry == NULL)
        {
            printf("Failed allocation.\n");
            //!!! failed_malloc();
            return NULL;
        }
        entry->key = hash_fn(prompt_names[i], num_buckets);
        //***my_strncpy(entry->name, prompt_names[i], NAMESIZE - 1); // i-1 to account for tar_name
        strncpy(entry->name, prompt_names[i], NAMESIZE - 1); // i-1 to account for tar_name
        entry->newest_version_flag = 1;
        entry->file_exists_flag = 0;
        entry->next = NULL;
        entry->mod_time = 0;

        if (add_entry(entry, names_table) < 0)
        {
            // print_error("Unable to add entry to hash table");
            printf("Unable to add entry to hash table");
            return NULL;
        }
    }
    return names_table;
}

// Search functionality

/************************************************************ */
/*
int name_found_in_tar_contents(int tar_fd, Hashtable *table)
 int num_prompt_names = table->num_prompt_names;

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
    if ((n < 0) && n != 512)
    {
        print_error("Unable to read magic tar file\n");
        return -1;
    }
    read_size += n;

    struct header *f_header = (struct header *)header_buffer;

    // Extracting the entire tar file
    if ((f_header->magic[0] == 'u' &&
         f_header->magic[1] == 's' &&
         f_header->magic[2] == 't' &&
         f_header->magic[3] == 'a' &&
         f_header->magic[4] == 'r' &&
         f_header->magic[5] == ' '))
    {

        if (num_prompt_names == 0)
        {
            print_error("my_tar command needs additional arguments to add files to tar file.");
            return -1;
        }

        else if (num_prompt_names > 0)
        {

       // vvv PUT IN FUNCTIONALITY FOR TO SKIP IF file_exists_flag = 0

        for (int i = 0; i < num_prompt_names; i++)
            {           //be aware that this will not pick up file names than 99 characters, since prefix is needed as well
                if (table->buckets[i]->file_exists_flag == 1 &&  (my_strcmp(table->buckets[i]->name, f_header->name) == 0))
                {
                    if(check_file_age(f_header->m_time, table->buckets[i]->name == 1)
                    {}
                    else
                    {
                        table->buckets[i]->newest_version_flag = 0;
                    }

                    break;
                }
            }
        }
    }
}

*/

// Checking file age vs. f_header file
/****************************************************** */
/*
//NOT SURE IF I WILL NEED TO BREAK OUT THE LOGIC HERE OR IF CAN JUST BE PART OF THE ORIGINAL CONDITOINAL

    time_t infile_mtime = (time_t)parse_octal(f_header->mtime, 12);

    time_t append_file


}
*/

int get_mod_times(Hashtable *table)
{
    if (table == NULL)
    {
        // print_error("Hashtable does not exist\n");
        printf("Hashtable does not exist\n");
        return -1;
    }

    for (int i = 0; i < table->num_buckets; i++)
    {
        if (table->buckets[i]->file_exists_flag == 1)
        {
            struct stat file_stats;
            if (stat(table->buckets[i]->name, &file_stats) == -1)
            {
                //*** file_error(table->buckets[i]->name);
                return -1;
            }
            table->buckets[i]->mod_time = file_stats.st_mtime;
        }
    }

    return 0;
}

int check_files_exist(Hashtable *table)
{
    if (table == NULL)
    {
        // print_error("Names hashtable does not exist\n");
        printf("Names hashtable does not exist\n");
        return -1;
    }

    int num_prompt_names = table->num_prompt_names;
    int num_buckets = table->num_buckets;

    DIR *dir;
    struct dirent *entry;

    if (num_prompt_names <= 0)
    {
        // print_error("No names found in names_table.\n");
        printf("No names found in names_table.\n");
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
    for (int i = 0; i < num_buckets && n < num_prompt_names; i++)
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
                        //!!! my_strcmp
                        if (strcmp(iterator->name, entry->d_name) == 0)
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

int main()
{
    int num_names = 5;
    char **names = malloc(sizeof(char *) * num_names);

    for (int i = 0; i < num_names; i++)
    {
        names[i] = malloc((NAMESIZE - 1) * sizeof(char));
    }
    //!!!mystrncpy
    strncpy(names[0], "test.tar", NAMESIZE - 1);
    strncpy(names[1], "file1.tar", NAMESIZE - 1);
    strncpy(names[2], "file2.tar", NAMESIZE - 1);
    strncpy(names[3], "file3.tar", NAMESIZE - 1);
    strncpy(names[4], "dir4", NAMESIZE - 1);

    for (int i = 0; i < num_names; i++)
    {
        printf("names[%d]: %s", i, names[i]);
    }

    Hashtable *table = build_prompt_names_table(names, num_names);
    if (check_files_exist(table) < 0)
    {
        // print_error("Error checking whether files indicated in hashtable exist.\n");
        printf("Error checking whether files indicated in hashtable exist.\n");
        return -1;
    }

    if (get_mod_times(table) < 0)
    {
        // print_error("Error finding modification times for prompt file names.\n");
        printf("Error finding modification times for prompt file names.\n");
        return -1;
    }

    printf("num_buckets: %ld\n num_prompt_names: %d\n", table->num_buckets, table->num_prompt_names);

    // for (int i = 0; i < num_names; i++)
    // {
    //     printf("bucket[%d] key: %d\n", i, table->buckets[i]->key);
    //     printf("bucket[%d] name: %s\n", i, table->buckets[i]->name);
    //     printf("bucket[%d] file_exists_flag: %d\n", i, table->buckets[i]->file_exists_flag);
    //     printf("bucket[%d] newest_version_flag: %d\n", i, table->buckets[i]->newest_version_flag);
    //     printf("bucket[%d] mod_time: %lld\n", i, (long long int)table->buckets[i]->mod_time);
    //     printf("\n");
    // }

    int n = 0;

    for (int i = 0; i < table->num_buckets && n < table->num_prompt_names; i++)
    {
        if (table->buckets[i] != NULL)
        {
            File_Entry *iterator = table->buckets[i];

            while (iterator != NULL)
            {
                {
                    printf("bucket[%d] key: %d\n", i, table->buckets[i]->key);
                    printf("bucket[%d] name: %s\n", i, table->buckets[i]->name);
                    printf("bucket[%d] file_exists_flag: %d\n", i, table->buckets[i]->file_exists_flag);
                    printf("bucket[%d] newest_version_flag: %d\n", i, table->buckets[i]->newest_version_flag);
                    printf("bucket[%d] mod_time: %lld\n", i, (long long int)table->buckets[i]->mod_time);
                    printf("\n");
                    n++;
                }
                iterator = iterator->next;
            }
        }
    }

    // CYCLE THRU UMAGICS TO COMPARE DATE MODIFIED
    // using hash function to find bucket and then
    // traverse until find node and compare DATE MODIFIED
    // both should be t_time

    // PRINT ERRORS
    // need to include the file not found print error functionality; I believe it should
    // have the two layers of errors... functionality that I should be able to copy

    for (int i = 0; i < 5; i++)
    {
        free(names[i]);
    }

    free(names);
    free_table(table);

    return 0;
}