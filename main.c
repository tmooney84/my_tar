/*
particular circumstances:
$ tar -c file1.c file2.txt dir3
>>tar: Refusing to write archive contents to terminal (missing -f option?)
>>tar: Error is not recoverable: exiting now
    these two lines are the response

  tar -cf
  tar -rf
  tar -tf
  tar -xf

  tar -tf -cf arc1
tar: -cf: Cannot open: No such file or directory
tar: Error is not recoverable: exiting now

needs to be a flags for c,r,t,x and f, + v flag. If one of those flags triggered the remainder of
args are files/directories ...except build in functionality for compression
gzip is a very common and fast compression algorithm.


gzip: tar -czvf my_archive.tar.gz my_directory
bzip2 generally provides better compression than gzip but is slower.
bzip2: tar -cjvf my_archive.tar.bz2 my_directory
//create my own compression later?

Exit Status >>> return 0 on success >0 if an error occurs (return 1)

Errors handling Errors will be written on STDERR.
File not found (provided file is: i_don_t_exist): my_tar: i_don_t_exist: Cannot stat: No such file or directory

Error with the tarball file (provided file is: tarball.tar): my_tar: Cannot open tarball.tar
*/

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

void flag_error();
void file_error(char *file_name);
void failed_alloc();
void print_string_array(char **all_names, int num_names);
void free_string_array(char **names, int num_names);
header *fill_header_info(char *file);

char **create_names_array(int argc, char **argv, int num_names);

int create_file(char *file_name, int flags, int perms);
int create_tar(char **names, int num_names); // int v_flag
int create_tar_file(char *tar_name, char op_flag);
int append_file_data(int tar_fd, char *append_file);
int update_tar(int argc, char **argv);
int list_tar(int argc, char **argv, int v_flag);
int extract_tar(char **names, int num_names); // int v_flag
int extract_all_contents(int tar_fd, char **names_to_extract, int num_ex_names);
int extract_process_entry(header *f_header, int tar_fd, int current_block);
int process_entry(char *path, int tar_fd);
int write_header(header *hdr, int tar_fd);
int write_file_data(int dst_fd, int src_fd, int f_size, int tar_flag);
int write_padding(int tar_fd, int total_required_padding);
int map_file_metadata(header *f_header, int fd);
int map_dir_metadata(header *f_header, char *file_name);
size_t parse_octal(char *str, size_t max_len);
char *parse_dir_slash(char *file_name);

// tar -czf -t >>> will throw error
// parse the files but if already c_flag, etc. is 1 then file_error(argv[i])
// if t then cannot have c, r, t, u, x ... but needs f
// -f to parse file correctly

int main(int argc, char **argv)
{
    // c,f,r,t,u,x            + v, z, j flags
    // int c_flag = 0;
    // int f_flag = 0;
    // int r_flag = 0;
    // int t_flag = 0;
    // int u_flag = 0;
    // int x_flag = 0;

    // int v_flag = 0;
    // int z_flag = 0;
    // int j_flag = 0;
    int num_flag_args = 1; // for now is 1 to make it work

    int num_names = argc - (num_flag_args + 1);
    char **first_name = argv;
    char **names = create_names_array(argc, first_name, num_names);
    if (!names)
    {
        failed_alloc();
        return -1;
    }

    if (argc == 1)
    {
        flag_error();
        return -1;
    }
    //*********************need to count the number of dashes and redo this section!!! tar -c -f  versus  tar -cf name.tar file_name
    else if (my_strcmp(argv[1], "-cf") == 0)
    {
        // int fd = create_file("zzz.txt", O_CREAT | O_RDWR, 0664);
        int fd = create_tar(names, num_names);
        if (fd < 0)
        {
            print_error("Error creating tar file\n");
            return -1;
        }

        // printf("fd successful if 0: %d\n", fd);
        //  if (create_tar(names, num_names) == -1) //int v_flag
        //  {
        //      return -1;
        //  }

        // TEST file_header_fns.c IN main:
        // tester_main(argv[2]);
        return 0;
    }
    // else if(my_strcmp(argv[1], "-cvf")==0)
    //     {
    //         v_flag = 1;
    //         create_tar(argc, argv, v_flag);
    //     }
    else if (my_strcmp(argv[1], "-rf") == 0)
    {
        // archive_tar(argc, argv);
    }
    else if (my_strcmp(argv[1], "-tf") == 0)
    {
        // t_flag = 1;
        // f_flag = 1;
        /*
        so for listing I need to cycle through each block of the record look for ustar and then if ustar
        my_printf the name; if a name is specified print that name as well.
       
        ***need to parse out the file names and use the same logic as found in create_tar just as open tar
        print_included_contents()
        int fd = create_tar(names, num_names);
        if (fd < 0)
        {
            print_error("Error creating tar file\n");
            return -1;
        }

        */
           
    }
    // else if(my_strcmp(argv[1], "-tvf")==0)
    //     {
    //         t_flag = 1;
    //         v_flag = 1;
    //         f_flag = 1;
    //     }
    else if (my_strcmp(argv[1], "-uf") == 0)
    {
        // u_flag = 1;
        // f_flag = 1;
    }
    else if (my_strcmp(argv[1], "-xf") == 0)
    {
        if (extract_tar(names, num_names))
        {
            print_error("Unable to extract tar file contents.\n");
            return -1;
        }
    }
    // else if(my_strcmp(argv[1], "-xvf")==0)
    //     {
    //         x_flag = 1;
    //         v_flag = 1;
    //         f_flag = 1;
    //     }

    else
    {
        flag_error();
        return -1;
    }

    // my_printf("argc: %d\n", argc);
    // my_printf("c: %d, f: %d, j: %d, r: %d, t: %d, u: %d, v: %d, x: %d, z: %d\n", c_flag, f_flag, j_flag, r_flag, t_flag, u_flag, v_flag, x_flag, z_flag);

    //*** NEED TO IMPLEMENT num_flag_args ABOVE  */

    // argv[2]
    // test whether string file or folder if not error

    //-cf -rf -uf      create archive? add to archive? add to archive if update (stat time conditional)? >>>
    // this could all be one fn w flags

    // print out archive contents >>> additional function

    // extract archive

    free_string_array(names, num_names);
    return 0;
}

void flag_error()
{
    print_error("tar: You must specify one of the '-Acdtrux', '--delete' or '--test-label' options\nTry 'tar --help' or 'tar --usage' for more information.\n");
}

void file_error(char *file_name)
{
    print_error("my_tar: %s: Cannot stat: No such file or directory\n", file_name);
}

void failed_alloc()
{
    print_error("Failed to allocate memory.\n");
}

void append_error()
{
    print_error("Failure to append file.\n");
}

void tarball_error(char *tar_name)
{
    print_error("my_tar: Cannot open %s\n", tar_name);
}

void file_not_found_error(char *file_name)
{
    print_error("tar: %s: Not found in archive\n", file_name);
}

void previous_errors()
{
    print_error("tar: Exiting with failure status due to previous errors\n");
}

void print_string_array(char **all_names, int num_names)
{
    for (int i = 0; i < num_names; i++)
    {
        my_printf("%s\n", all_names[i]);
    }
}

// frees pointers related to string array
void free_string_array(char **names, int num_names)
{
    if (names == NULL)
    {
        return;
    }

    for (int i = 0; i < num_names; i++)
    {
        free(names[i]); // Free each dynamically allocated string
    }

    free(names);
}

char **create_names_array(int argc, char **argv, int num_names)
{
    char **names = malloc((num_names) * sizeof(char *));
    if (!names)
    {
        failed_alloc();
        return NULL;
    }
    my_memset(names, 0, (num_names) * (sizeof(char *)));
    int names_index = 0;

    for (int i = argc - num_names; i < argc; i++)
    {
        int names_len = my_strlen(argv[i]);
        names[names_index] = malloc((my_strlen(argv[i]) + 1) * sizeof(char));
        if (!names[names_index])
        {
            failed_alloc();
            return NULL;
        }
        my_memset(names[names_index], 0, names_len);

        my_strncpy(names[names_index], argv[i], names_len);
        names_index++;
    }

    // print_string_array(names, names_index);
    return names;
}
// 346-416 commented for testing

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

int create_tar_file(char *tar_name, char op_flag)
{
    int tar_fd;

    // create tar file:
    if (op_flag == 'c')
    {
        tar_fd = create_file(tar_name, O_RDWR | O_CREAT | O_TRUNC, TAR_PERMS);
    }

    if (op_flag == 't')
    {
        tar_fd = create_file(tar_name, O_RDONLY , TAR_PERMS);
    }

    else if (op_flag == 'r' || op_flag == 'u')
    {
        tar_fd = create_file(tar_name, O_RDWR | O_CREAT | O_APPEND, TAR_PERMS);
    }
    // tar_fd = create_file(tar_name, O_CREAT, TAR_PERMS);//TAR_PERMS
    // printf("tar_fd: %d\n", tar_fd);

    else
    {
        tarball_error(tar_name);
    }

    if (tar_fd < 0)
    {
        tarball_error(tar_name);
        return -1;
    }
    return tar_fd;
}

int create_tar(char **names, int num_names) // int v_flag
{
    int tar_fd;

    char *tar_name = names[0];
    // printf("tar_name: %s\n", tar_name);
    tar_fd = create_tar_file(tar_name, 'c');

    if (tar_fd < 0)
    {
        return -1;
    }

    for (int i = 1; i < num_names; i++)
    {
        // printf("test\n");
        if (process_entry(names[i], tar_fd) < 0)
        {
            my_printf("Error processing %s into tar file\n", names[i]);
            return 1;
        }
    }

    // add_zeros(tar_fd);

    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;
    // printf("tar_size before padding: %ld\n", tar_size);

    // need two zero blocks and then need to see if that goes over the size of a record
    int zero_padding = 2 * BLOCKSIZE;
    int padded_data = tar_size + zero_padding;
    int total_required_padding;

    int rec_num = (tar_size % (RECORDSIZE * BLOCKSIZE) == 0) ? tar_size / (RECORDSIZE * BLOCKSIZE) : tar_size / (RECORDSIZE * BLOCKSIZE) + 1;

    int rec_num_wpad = ((padded_data) % (RECORDSIZE * BLOCKSIZE) == 0) ? padded_data / (RECORDSIZE * BLOCKSIZE) : padded_data / (RECORDSIZE * BLOCKSIZE) + 1;

    if (rec_num == rec_num_wpad)
    {
        total_required_padding = rec_num * (RECORDSIZE * BLOCKSIZE) - tar_size;
    }
    else
    {
        total_required_padding = rec_num_wpad * (RECORDSIZE * BLOCKSIZE) - tar_size;
    }

    // printf("padding needed: %d\n", total_required_padding);

    if (write_padding(tar_fd, total_required_padding) < 0)
    {
        print_error("Unable to add padding\n");
        return -1;
    }

    tar_size = (long int)tar_stats.st_size;
    // printf("tar_size after padding added: %ld\n", tar_size);

    close(tar_fd);

    return 0;
}

int process_entry(char *path, int tar_fd)
{
    struct stat arg_stats;
    if (stat(path, &arg_stats) < 0)
    {
        file_error(path);
    }
    // tester_main(path);
    header *hdr = fill_header_info(path);

    // printf("tar_fd: %d\n", tar_fd);
    if (!hdr)
    {
        file_error(path);
        return -1;
    }
    write_header(hdr, tar_fd);

    free(hdr);

    if (S_ISREG(arg_stats.st_mode))
    {
        // if file to append
        if (append_file_data(tar_fd, path) != 0)
        {
            file_error(path);
            return -1;
        }
    }

    else if (S_ISDIR(arg_stats.st_mode))
    {
        DIR *dir = opendir(path);
        if (!dir)
        {
            file_error(path);
            return -1;
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            if (my_strcmp(entry->d_name, ".") == 0 || my_strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            char rel_path[PATH_MAX];
            my_memset(rel_path, 0, PATH_MAX);

            int entry_name_len = my_strlen(entry->d_name);
            int path_len = my_strlen(path);

            my_strncpy(rel_path, path, path_len);
            rel_path[path_len] = '/';
            my_strncpy(rel_path + path_len + 1, entry->d_name, entry_name_len);

            // printf("Relative Path Name: %s\n", rel_path);
            if (process_entry(rel_path, tar_fd) < 0)
            {
                print_error("Failure to process directory entries\n");
                closedir(dir);
                return -1;
            }
        }

        closedir(dir);

        //    if(v_flag) >>> to print the file that was added
        // {
        //     my_printf("%s\n", names[i]);
        // }
    }

    return 0;
}

int extract_tar(char **names, int num_names) // int v_flag
{
    int tar_fd;

    char *tar_name = names[0];
    // printf("tar_name: %s\n", tar_name);
    tar_fd = open(tar_name, O_RDONLY, TAR_PERMS);

    if (tar_fd < 0)
    {
        tarball_error(tar_name);
        return -1;
    }

    if (num_names > 1)
    {
        for (int i = 1; i < num_names; i++)
        {
            // names + 1: truncates list to only have names of desired files and directories for extraction
            // num_names - 1: number of names of desired files and directories for extraction
            if (extract_all_contents(tar_fd, names + 1, num_names - 1) < 0)
            {
                print_error("Error processing %s into tar file\n", names[i]);
                return 1;
            }
        }
    }
    else
    {
        if (extract_all_contents(tar_fd, NULL, 0) < 0)
        {
            print_error("Error processing extracting contents from tar file.\n");
        }
    }

    close(tar_fd);

    return 0;
}

/*
For Special Files and Links:

Create the File/Directory/Link:
Use the name (and prefix) fields to create the file or directory. For example:

For a regular file, use open()/creat() and write the data.
For a directory, use mkdir().
For a symlink, use symlink() with the linkname field.


Special Files:

Device Files: If the file is a character or block device (as indicated by the typeflag), use mknod() with the appropriate major and minor numbers.
FIFO (Named Pipe): Use mkfifo() to create a FIFO.
Symlinks: Use symlink() to create a symbolic link, using the header’s linkname as the target.

mapping to struct stat:
For device files, st_rdev is set using devmajor and devminor
*/

int extract_all_contents(int tar_fd, char **names_to_extract, int num_ex_names)
{
    struct stat tar_stats;
    if (fstat(tar_fd, &tar_stats) == -1)
    {
        print_error("Unable to stat tar\n");
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;
    int total_blocks = tar_size / BLOCKSIZE;
    if (tar_size % BLOCKSIZE != 0)
    {
        print_error("Error non-uniform tar size\n");
        return -1;
    }

    int current_block = 0;

    // makes sure tar_fd is at beginning of the file
    if (lseek(tar_fd, 0, SEEK_SET) < 0)
    {
        print_error("Unable to lseek file\n");
        return -1;
    }

    unsigned char header_buffer[512];

    while (current_block < total_blocks)
    {
        // added to make sure aligned on correct block
        lseek(tar_fd, current_block * 512, SEEK_SET);
        my_memset(header_buffer, 0, sizeof(header_buffer));
        int n = 0;
        int returned_blocks = 0;

        // set block to current location ???
        // lseek(tar_fd, current_block * 512, SEEK_SET);

        // if ((n = read(tar_fd + (current_block * 512), header_buffer, 512) < 0) && n != 512)
        n = read(tar_fd, header_buffer, 512);
        if ((n < 0) && n != 512)
        {
            print_error("Unable to read magic tar file\n");
            return -1;
        }
        current_block++;

        struct header *f_header = (struct header *)header_buffer;

        // Extracting the entire tar file
        if ((f_header->magic[0] == 'u' &&
             f_header->magic[1] == 's' &&
             f_header->magic[2] == 't' &&
             f_header->magic[3] == 'a' &&
             f_header->magic[4] == 'r' &&
             f_header->magic[5] == ' ') &&
            (num_ex_names == 0))
        {
            // do I need written_blocks?
            if ((returned_blocks = extract_process_entry(f_header, tar_fd, current_block)) < 0)
            {
                print_error("Error... unable to extract file from tar\n");
                return -1;
            }
            current_block = returned_blocks;
            continue;
        }

        // Extracting specific file names
        if (my_strcmp(f_header->magic, "ustar") == 0 && (num_ex_names != 0))
        {
            {
                for (int i = 0; i < num_ex_names; i++)
                {
                    if ((names_to_extract[i] != 0) && my_strcmp(f_header->name, names_to_extract[i]) == 0)
                    {
                        if ((returned_blocks = extract_process_entry(f_header, tar_fd, current_block)) < 0)
                        {
                            print_error("Error... unable to extract file from tar\n");
                            return -1;
                        }
                        names_to_extract[i] = 0;

                        current_block = returned_blocks;
                    }
                }
            }
        }
    }

    // prints errors for those file names from command that are not found

    if (num_ex_names > 0)
    {
        int error_flag = 0;
        for (int i = 0; i < num_ex_names; i++)
        {
            if (names_to_extract[i] == 0)
            {
                continue;
            }

            file_not_found_error(names_to_extract[i]);
            error_flag = 1;
        }
        if (error_flag == 1)
        {
            previous_errors();
        }
        //!!! Temporary
        else
        {
            printf("No errors with prompted names\n");
        }
        //
    }
    return 0;
}

int extract_process_entry(header *f_header, int tar_fd, int current_block)
{
    char file_name[NAMESIZE + 155];

    int name_len = my_strlen(f_header->name);

    if (f_header->prefix[0] != '\0')
    {
        int prefix_len = my_strlen(f_header->prefix);
        my_strncpy(file_name, f_header->prefix, prefix_len);
        if(f_header->prefix[prefix_len -1] != '/')
        {
            file_name[prefix_len] = '/';
            prefix_len++;
        }
        my_strncpy(file_name + prefix_len, f_header->name, name_len);
    }

    else
    {
        my_strncpy(file_name, f_header->name, NAMESIZE);
    }
    int file_flags = O_RDWR | O_CREAT | O_TRUNC;
    int file_perms = (int)parse_octal(f_header->mode, sizeof(f_header->mode));
    char file_type = f_header->typeflag;
    long int file_size = (long int)parse_octal(f_header->size, sizeof(f_header->size));

    // if reg file or symbolic link 0, 2  // need to check how this covers symbolic links
    if (file_type == '0' || file_type == '2')
    {
        int fd = create_file(file_name, file_flags, file_perms);
        if (fd < 0)
        {
            my_printf("Unable to create %s", file_name);
            return -1;
        }
        if (fd >= 0)
        {
            if (fchmod(fd, file_perms) == -1)
            {
                print_error("fchmod failed");
                return -1;
            }
        }

        long int num = 0;
        int num_blocks = 0;

        num = write_file_data(fd, tar_fd, file_size, 0);
        if ((num < 0) && num != file_size)
        {
            print_error("Unable to extract file contents\n");
            return -1;
        }

        if (map_file_metadata(f_header, fd) < 0)
        {
            print_error("unable to map header data to file stat\n");
            return -1;
        }

        num_blocks = num % BLOCKSIZE != 0 ? num / BLOCKSIZE + 1 : num / BLOCKSIZE;

        current_block += num_blocks;
        close(fd);
    }

    // if directory
    else if (file_type == '5')
    {
        mode_t dir_mode = (mode_t)(0040000 | parse_octal(f_header->mode, sizeof(f_header->mode)));

        char *dir_name = parse_dir_slash(file_name);

        if (mkdir(dir_name, dir_mode) < 0)
        {
            print_error("mkdir failed\n");
            return -1;
        }
        if (chmod(dir_name, file_perms) == -1)
        {
            print_error("fchmod failed");
            return -1;
        }

        if (map_dir_metadata(f_header, dir_name) < 0)
        {
            print_error("unable to map header data to file stat\n");
            return -1;
        }

        free(dir_name);
    }

    return current_block;
}

char *parse_dir_slash(char *file_name)
{
    int file_name_size = my_strlen(file_name);
    char *dir_name = malloc(sizeof(NAMESIZE));
    if (!dir_name)
    {
        print_error("to create dir_name in parse_dir_slash()\n");
        return NULL;
    }

    my_memset(dir_name, 0, NAMESIZE);
    int i = 0;
    for (; i < file_name_size; i++)
    {
        dir_name[i] = file_name[i];
    }

    if (file_name[file_name_size - 1] == '/')
    {
        dir_name[file_name_size - 1] = '\0';
    }

    return dir_name;
}

/******************* */
int map_file_metadata(header *f_header, int fd)
{
    struct stat file_stats;
    if (fstat(fd, &file_stats) == -1)
    {
        // may need to create ffile_error()
        print_error("Cannot stat tar file*** may need ffile_error");
        return -1;
    }

    //** need to make sure mode has the compressed permissions and file type */
    // switch (f_header->typeflag)
    // {
    // case '0':
    //     file_stats.st_mode = (mode_t)0100000 || parse_octal(f_header->mode, sizeof(f_header->mode));
    //     if(fchmod(fd, file_stats.st_mode) < 0)
    //     {
    //         print_error("Unable to set mode\n");
    //         return 1;
    //     }
    //     break;

    // // case '2': //symbolic link
    // //     file_stats->st_mode = (mode_t)0120000 || parse_octal(f_header->mode, sizeof(f_header->mode));
    // // if(fchmod(fd, file_stats->st_mode) < 0)
    // //     {
    // //         print_error("Unable to set mode");
    // //         return 1;
    // //     }
    // //     break;

    // default:
    //     // file_stats.st_mode = '\0';
    //     print_error("Unable to set mode\n");
    //     break;
    // }

    ///...if others needed get from fill_typeflag

    file_stats.st_uid = (unsigned int)parse_octal(f_header->uid, sizeof(f_header->uid));
    file_stats.st_gid = (unsigned int)parse_octal(f_header->gid, sizeof(f_header->gid));
    if (fchown(fd, file_stats.st_uid, file_stats.st_gid) < 0)
    {
        print_error("Unable to set file ownership\n");
        return -1;
    }
    // file_stats.st_size = lseek(fd, 0, SEEK_END); // could do parse_octal(f_header->size, 12);

    // struct timespec times[2];
    // file_stats.st_mtime = (time_t)parse_octal(f_header->mtime, sizeof(f_header->mtime));
    // times[1].tv_sec = file_stats.st_mtime;

    /* REMEMBER OCT STRING TO INT
   st_mode    chmod()   → Derived from the tar header’s mode and typeflag
st_uid and st_gid    chown()   → Derived from the header’s uid and gid
st_size → Tells you how many bytes of file data to read
st_mtime    utime()    futime()   → Derived from the header’s mtime

map_file_data(f_header, fd)
*/
    return 0;
}

int map_dir_metadata(header *f_header, char *file_name)
{
    struct stat file_stats;
    if (stat(file_name, &file_stats) == -1)
    {
        return -1;
    }

    //** need to make sure mode has the compressed permissions and file type */
    // switch (f_header->typeflag)
    // {
    // case '0':
    //     file_stats.st_mode = (mode_t)0100000 || parse_octal(f_header->mode, sizeof(f_header->mode));
    //     if (chmod(file_name, file_stats.st_mode) < 0)
    //     {
    //         print_error("Unable to set mode\n");
    //         return 1;
    //     }
    //     break;

    // case '2': //symbolic link
    //     file_stats->st_mode = (mode_t)0120000 || parse_octal(f_header->mode, sizeof(f_header->mode));
    // if(fchmod(fd, file_stats->st_mode) < 0)
    //     {
    //         print_error("Unable to set mode");
    //         return 1;
    //     }
    //     break;

    // default:
    //     // file_stats.st_mode = '\0';
    //     print_error("Unable to set mode\n");
    //     break;
    // }

    ///...if others needed get from fill_typeflag

    file_stats.st_uid = (unsigned int)parse_octal(f_header->uid, sizeof(f_header->uid));
    file_stats.st_gid = (unsigned int)parse_octal(f_header->gid, sizeof(f_header->gid));
    if (chown(file_name, file_stats.st_uid, file_stats.st_gid) < 0)
    {
        print_error("Unable to set file ownership\n");
        return -1;
    }
    // file_stats.st_size = lseek(fd, 0, SEEK_END); // could do parse_octal(f_header->size, 12);

    // struct timespec times[2];
    // file_stats.st_mtime = (time_t)parse_octal(f_header->mtime, sizeof(f_header->mtime));
    // times[1].tv_sec = file_stats.st_mtime;

    /* REMEMBER OCT STRING TO INT
   st_mode    chmod()   → Derived from the tar header’s mode and typeflag
st_uid and st_gid    chown()   → Derived from the header’s uid and gid
st_size → Tells you how many bytes of file data to read
st_mtime    utime()    futime()   → Derived from the header’s mtime

map_file_data(f_header, fd)
*/
    return 0;
}

size_t parse_octal(char *str, size_t max_len)
{
    size_t num = 0;
    size_t i = 0;
    for (i = 0; i < max_len && str[i] >= '0' && str[i] <= '7'; ++i)
    {
        num *= 8;
        num += str[i] - '0';
    }

    return num;
}

int write_padding(int tar_fd, int total_required_padding)
{
    char zero_buff[total_required_padding];
    my_memset(zero_buff, 0, total_required_padding);

    ssize_t bytes_written = 0;

    int end_data = lseek(tar_fd, 0, SEEK_END);
    if (end_data < 0)
    {
        print_error("Unable to random access tar file\n");
        return -1;
    }
    // printf("End data: %d\n", end_data);

    while (bytes_written < total_required_padding)
    {
        ssize_t written = write(tar_fd, zero_buff + bytes_written, total_required_padding - bytes_written);
        if (written < 0)
        {
            print_error("Failure to write data.\n");
            return -1;
        }
        bytes_written += written;
        // printf("writing data\n");
    }
    // printf("bytes_written: %zu\n", bytes_written);
    return 0;
}

int append_file_data(int tar_fd, char *append_file)
{
    // printf("append_file_data named %s started!!!!!!\n", append_file);
    //  get file size
    struct stat file_stats;

    // what to do about symbolic links lsat and in tar??
    if (stat(append_file, &file_stats) == -1)
    {
        return -1;
    }

    long int f_size = (long int)file_stats.st_size;
    // printf("f_size in append: %ld\n", f_size);

    // get tar size
    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;
    // printf("tar_size in append: %ld\n", tar_size);

    int append_fd = open(append_file, O_RDONLY);

    if (tar_size < BLOCKSIZE)
    {
        print_error("Failure to write file header\n");
        return -1;
    }

    // if the tar is larger than size of header then just append because
    else
    {
        int ts_n;

        ts_n = write_file_data(tar_fd, append_fd, f_size, 1);

        if (ts_n < 0)
        {
            print_error("Unable to append file data\n");
            return -1;
        }
    }

    close(append_fd);
    // printf("closed append_fd\n");
    return 0; // if successful may need conditional logic
}

int write_header(header *hdr, int tar_fd)
{
    unsigned char *hdr_data = (unsigned char *)hdr;
    size_t bytes_written = 0;

    while (bytes_written < BLOCKSIZE)
    {
        // ssize_t >>> [-1, SIZE_MAX] bytes, if issue returns -1
        ssize_t written = write(tar_fd, hdr_data + bytes_written, BLOCKSIZE);
        if (written < 0)
        {
            print_error("write_header: write failed\n");
            return -1;
        }
        bytes_written += written;
    }
    // printf("bytes_written: %ld\n", bytes_written);
    // printf("header written\n");
    return 0;
}


/* 
write_file_data(): logic is for the partial writes in the events of system buffering and interrupts. 
It a more common in pipes,fifos and sockets than regular files, but can possibly happen during larger
file writes.
*/
 int write_file_data(int dst_fd, int src_fd, int f_size, int tar_flag)
{
    // printf("write_file_data starting...\n");
    unsigned char transfer_buff[BLOCKSIZE];
    ssize_t total_bytes_written = 0;
    ssize_t additional_size = 0;
    ssize_t n = 0;
    ssize_t add_written = 0; // taken from the bottom conditional to carry through at end total
    
    while (1)
    {
        ssize_t bytes_to_read = 0;
        //^^^ replaced with 0
        // ssize_t bytes_to_read = BLOCKSIZE;

        // calculates read size
        if (f_size > 0)
        {
            ssize_t remaining = f_size - total_bytes_written;
            if (remaining <= 0)
                break; // all data read
            bytes_to_read = (remaining < BLOCKSIZE) ? remaining : BLOCKSIZE;
        }

        if ((n = read(src_fd, transfer_buff, bytes_to_read)) < 0)
        {
            print_error("read error\n");
            return -1;
        }

        if (n == 0)
        {
            break; // EOF reached
        }

        // handles partial writes
        ssize_t bytes_written = 0;

        while (bytes_written < n)
        {
            ssize_t written = write(dst_fd, transfer_buff + bytes_written, n - bytes_written);
            if (written < 0)
            {
                print_error("Failure to write file data\n");
                return -1;
            }
            bytes_written += written;
        }
        total_bytes_written += n;

        //if writing to tar file add intra-block padding
        if(tar_flag == 1)
        {
        if (total_bytes_written % BLOCKSIZE != 0)
        {
            additional_size = BLOCKSIZE - (total_bytes_written % BLOCKSIZE);
        }

        if (additional_size > 0)
        {
            unsigned char add_buff[additional_size];
            my_memset(add_buff, '\0', additional_size);

            while (add_written < additional_size)
            {
                ssize_t written = write(dst_fd, add_buff + add_written, additional_size - add_written);
                if (written < 0)
                {
                    print_error("Failure to write padding\n");
                    return -1;
                }
                add_written += written;
            }
        }
        }
   }
    int write_size = (long int)total_bytes_written + (long int)add_written;

    return write_size;
}
// +11 lines are from debugging from write_file_data
// printf("additional bytes_written: %d\n", add_written);
// printf("GRAND TOTAL FOR FILE bytes_written: %ld\n", total_bytes_written + add_written);

// struct stat tar_stats;

// if (fstat(tar_fd, &tar_stats) == -1)
// {
//     return -1;
// }

// long int tar_size = (long int)tar_stats.st_size;
// printf("tar_size in write_file_data: %ld\n", tar_size);

//        //check if is file vs dir
//        file_header_info(names[i]);

//        if(names[i] filetype is a file)
//        {
//            append_file();  >>> includes  fill_header(names[i]);>>> MAKE SURE TO START APPENDING WRITE WHEN ZERO PADDING STARTS AND MAKE

//            ADJUSTMENTS ACCORDINGLY
//        }
//        //if dir then need to go in recursively
//        {
//        recursive into directory and sub-directories
//        {

//        if(names[i] filetype is a direcory)
//                    (recursively for files in folders and sub_folders)
//        {
//            file_header_info(directory_name);

//            do the same for each file in the directory... recursively
//            file_header_info(file_name);
//            append_file() >>> for each file in the directories
//        }
//         }
//            }
//    }

//    //add two zero blocks to end of file >>> this could be part of the append_file function at end
//    add_zero_block(tar_file_name.tar);
//    add_zero_block(tar_file_name.tar);
//    logic to make sure that the bocks and records align properly

//

/*
determine total size of files + size of headers + 2 "00" 512bytes with padding to determine
how many records are needed (20 blocks/ record)

1) -create file with the supplied name
function: create_file(char *file_name)

could be more efficient if do a first sweep through gathering the data and building temporary
tars

file 1 ... file 2 ... file 3

-get info and fill header for file 1, same for file2 and file3... take the size field convert
to int (size1 + size2 + size3 + 3 x FH_SIZE is this < 20 blocks use % and if not exact fix need pad the
last record and then an addional 2 512 "00" blocks into an additional record)

sub-functions: fill_header_info(char * file_name) >>> need think about how this will be run recursively for folders
           calc_num_records (int file_sizes[argc - non_file_args]) >>> remember int size = sizeof(myArray) / sizeof(myArray[0]);
           add_zero_block(tar_file_name.tar);
           append_file(char *names)



challenges: need to research how to do this recursively for folders

2) after gathering file info into header blocks begin to copy the file byte-by-byte
into each block until filled, if not exact fit into last block of record >>> add padding

sub-functions:
append_tar(char *tar_file, char *append_file_name) >>> can reuse with update_file fn, just need to include
-r flag or do quazi overload... update_file_name >>> same logic just with -r to do conditional on update

verbose printing of file completed need to test -v

add zero blocks record to end of file
^^^ this may be a sub-function

verbose printing of tar completed >>> need to test -v

return 0 success 1 for failure
*/

// int archive_tar(int argc, char **argv)

// // append_tar(char **names, int num_names) >>> first of the list should be the tar file
// {
//     /*
//     append_file(char **names, int num_names); >>> includes the 2 x zero blocks
//     //add two zero blocks to end of file >>> this could be part of the append_file function at end
//     add_zero_block(tar_file_name.tar);
//     add_zero_block(tar_file_name.tar);
//     logic to make sure that the bocks and records align properly
// }

// int update_tar(int argc, char **argv)
// {
// if(new_file.modification_time > old_file.modification_time)
// {
// append_file() >>> with same logic as append_tar
// }
// }
// update/append_file(char *tar_file, char *append_file_name) >>> can reuse with update_file fn, just need to include
// -r flag or do quazi overload... update_file_name >>> same logic just with -r to do conditional on update

// */
// }

// int list_tar(int argc, char **argv, int v_flag)
// {
//     /*cycle through testing whether type is file or directory, if directory add '/' to end when
//     print to screen; rely '/' on the dir1/file3.txt to skip to know
//     whether it is a file or a path
//     */
// }

// int extract_tar(int argc, char **argv, int v_flag)
// {
//     /*
//     cycle through file to find headers until hit the two zero blocks
//        until end of tar
//         if file: create(overwrite?) file after fast forwarding through to file beginning in tar and
//         run the copy as far as the end of the archived file

//         if dir: create dir with header ...contained files should be named "dir1/file1.txt" so should
//         picked up by the previous logic
//     */
// }

// // adding verbose to these just have a print and with ifv_flag)

// // zip would be additional logic>>> some form of api
