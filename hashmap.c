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
#define MAX_FILENAME 255

#define PATH_MAX 4096
// better to use a union? padding needed?

#define TAR_PERMS 0664 // RW for owner, group & R for others

#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2

typedef struct
{
    int key;
    char name[MAX_FILENAME];
    int newest_version_flag;
    int file_exists_flag;
    File_Entry *next;
} File_Entry;

typedef struct
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

    //File_Entry *table_data = (File_Entry *)table;

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
        print_error("Unable to add name to hash table");
        return -1;
    }

    return 0;
}

// free table data
void free_table(Hashtable *table)
{
    if (table == NULL)
    {
        return;
    }
    for (size_t i = 0; i < table->num_buckets; i++)
    {
        // is it * or not
        free(table->buckets[i]); //>>> need to traverse and free each pointer
    }
    free(table);
}

int hash_fn(char *name, int num_buckets)
{
    int sum = 0;

    for (int i = 0; i < my_strlen(name); i++)
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
    Hashtable *prompt_names_table = create_table(num_buckets);
    prompt_names_table->num_prompt_names = num_prompt_names;
    for (int i = 0; i < num_prompt_names; i++)
    {
        File_Entry *entry = malloc(sizeof(File_Entry *));
        if (entry == NULL)
        {
            failed_malloc();
            return -1;
        }
        entry->key = hash_fn(prompt_names[i], num_buckets);
        my_strncpy(entry->name, prompt_names[i], MAX_FILENAME); // i-1 to account for tar_name
        entry->newest_version_flag = 1;
        entry->file_exists_flag = 0;
        entry->next = NULL;

        if(add_entry(entry, prompt_names_table) < 0)
        {
            //print_error("Unable to add entry to hash table");
            printf("Unable to add entry to hash table");
            return -1;
        }
    }
    return prompt_names_table;
}

// Search functionality

/************************************************************ */
/*
int name_found_in_tar_contents(int tar_fd, Hashtable *prompt_names_table)
 int num_prompt_names = prompt_names_table->num_prompt_names;

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
                if (prompt_names_table->buckets[i]->file_exists_flag == 1 &&  (my_strcmp(prompt_names_table->buckets[i]->name, f_header->name) == 0))
                {
                    if(check_file_age(f_header->m_time, prompt_names_table->buckets[i]->name == 1)
                    {}
                    else
                    {
                        prompt_names_table->buckets[i]->newest_version_flag = 0; 
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

int check_file_age(f_header->m_time, char *prompt_names_table->buckets[i]->name)
{

}

int check_files_exist(Hashtable names_table)
{
    int num_prompt_names = table->num_prompt_names;
    names_table 

    DIR *dir;
    struct dirent *entry



//DO I NEED DIRECTORY PATH?????!!!!

//char **return_directory(const char *dir_path, int a_flag, int file_count)
int check_files_exist(Hashtable names_table)
{
    int num_prompt_names = table->num_prompt_names;
    
    DIR *dir;
    struct dirent *entry;

    if (num_prompt_names <= 0) 
    {
        print_error("No names found in names_table.\n");
        return -1;
    }

    if ((dir = opendir(dir_path)) == NULL)
    {
        perror("Error opening directory");
        free(names);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        for(int i = 0; i < num_prompt_names; i++)
        {
        if(my_strcmp(table->buckets[i]->name, entry) == 0)
        {
            table->buckets[i]->file_exists_flag = 1;
        }
        }
        closedir(dir);
        return 0;
    }
    closedir(dir);

    string_quickSort(names, 0, file_count - 1);

    return names;
}













    }


*/


int main()
{
    int num_names = 5;
    char **names = malloc(sizeof(char *) * num_names);

    for (int i = 0; i < num_names; i++)
    {
        names[i] = malloc(255 * sizeof(char));
    }

    names[0] = "test.tar";
    names[1] = "file1.txt";
    names[2] = "file2.txt";
    names[3] = "file3.txt";
    names[4] = "dir4";

    for (int i = 0; i < num_names; i++)
    {
        printf("names[%d]: %s", i, names[i]);
    }

    Hashtable *prompt_names_table = build_prompt_names_table(names, num_names);
    if(check_files_exist(prompt_names_table) < 0)
    {
        print_error("Error checking whether files indicated in hashtable exist.\n");
        return -1;
    }
        
    printf("num_buckets: %d\n num_prompt_names: %d\n",prompt_names_table->num_buckets, prompt_names_table->num_prompt_names);

    for (int i = 0; i < num_names; i++)
    {
        printf("bucket[%d] key: %d\n", i, prompt_names_table->buckets[i]->key);
        printf("bucket[%d] name: %d\n", i, prompt_names_table->buckets[i]->name);
        printf("bucket[%d] file_exists_flag: %d\n", i, prompt_names_table->buckets[i]->file_exists_flag);
        printf("bucket[%d] newest_version_flag: %d\n", i, prompt_names_table->buckets[i]->newest_version_flag);
        printf("\n");
    }


    for (int i = 0; i < 5; i++)
    {
        free(names[i]);
    }

    free(names);
    free_table(prompt_names_table);

    return 0;
}