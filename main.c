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
#include "tar_utils.h"

#define RECORDSIZE 20
#define NAMESIZE 100
#define TUNMLEN 32
#define TGNMLEN 32
#define PREFIXSIZE 155
#define BLOCKSIZE 512
#define MAX_FILENAME 255
#define PATH_MAX 4096
#define TAR_PERMS 0664 // RW for owner, group & R for others
#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        flag_error();
        return -1;
    }

    else if(argc == 2)
    {
        print_error("tar: option requires an argument -- 'f'");
        return -1;
    }

    int num_flag_args = 1; // for now is 1 to make it work
    
    int num_names = argc - (num_flag_args + 1);
    char **first_name = argv;
    char **names = create_names_array(argc, first_name, num_names);
    if (!names)
    {
        failed_alloc();
        return -1;
    }


    else if (my_strcmp(argv[1], "-cf") == 0)
    {
        int fd = create_tar(names, num_names);
        if (fd < 0)
        {
            print_error("Error creating tar file\n");
            free_string_array(names, num_names);
            return -1;
        }
    }
    else if (my_strcmp(argv[1], "-rf") == 0)
    {
        char *tar_name = names[0];

        char **archive_names = names + 1;
        int num_archive_names = num_names -1;

        char op_flag = 'r';
        if (archive_tar(tar_name, archive_names, num_archive_names, op_flag) < 0)
        {
            print_error("Unable to archive files");
            free_string_array(names, num_names);
            return -1;
        }

        tar_name = NULL;
        archive_names = NULL;
    }
    else if (my_strcmp(argv[1], "-tf") == 0)
    {
        int tar_fd = open_tar(names);
        if (tar_fd < 0)
        {
            print_error("Error creating tar file\n");
            free_string_array(names, num_names);
            return -1;
        }

        if (print_included_tar_contents(tar_fd, names, num_names) < 0)
        {
            print_error("Unable to print contents of tar file");
            free_string_array(names, num_names);
            return -1;
        }
    }
    else if (my_strcmp(argv[1], "-uf") == 0)
    {
        char op_flag = 'u';

        // printf("NAMES GOING TO HASHMAP:\n");
        // for(int i = 0; i < num_names; i++)
        // {
        //     printf("names[%d]: %s\n", i, names[i]);
        // }
        //printf("\n");

        Hashtable *table = get_update_names(names, num_names);
        if(table == NULL)
        {
            failed_alloc();
            free_string_array(names, num_names);
            return -1;
        }

        Names_List *newest_names = get_newest_names(table);
        if (newest_names == NULL)
        {
            //FOR TESTING...
            printf("No files added with update function.");
            free_table(table); 
            free_string_array(names, num_names);
            return 0;
        }

        int num_update_names = newest_names->num_names;
        char **update_names = newest_names->names;

        /*****************PRINT OUT TEST************************/
        printf("NAMES COMING FROM HASHMAP GOING TO archive_tar():\n");
        for (int i = 0; i < num_update_names; i++)
        {
            printf("newest_names[%d]: %s\n", i, update_names[i]);
        }
        /*******************************************************/

        char *tar_name = names[0];
        
        if (archive_tar(tar_name, update_names, num_update_names, op_flag) < 0)
        {
            print_error("Unable to archive files");
            free_string_array(names, num_names);
            free_table(table); 
            return -1;
        }

        // print out error messages of incorrect file names
        print_error_names(table);

        free_names_list(newest_names);
        newest_names = NULL;

        free_table(table);
        table = NULL;

        update_names = NULL;
    }
    else if (my_strcmp(argv[1], "-xf") == 0)
    {
        if (extract_tar(names, num_names))
        {
            print_error("Unable to extract tar file contents.\n");
            free_string_array(names, num_names);
            return -1;
        }
    }
    else
    {
        flag_error();
        free_string_array(names, num_names);
        return -1;
    }

    free_string_array(names, num_names);
    names = NULL;
    return 0;
}
