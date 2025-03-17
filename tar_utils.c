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
// better to use a union? padding needed?

#define TAR_PERMS 0664 // RW for owner, group & R for others

#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2

int archive_tar(char *tar_name, char **archive_names, int num_archive_names, char op_flag) // int v_flag
{
    int tar_fd;

    tar_fd = create_tar_file(tar_name, op_flag);
    if (tar_fd < 0)
    {
        return -1;
    }

    int prev_error_flag = 0;

    struct stat tar_stats;
    if (fstat(tar_fd, &tar_stats) == -1)
    {
        print_error("Unable to stat tar\n");
        return -1;
    }

    // long int tar_size = (long int)tar_stats.st_size;

    unsigned char header_buffer[512];
    // int read_size = 0;

    // begin search for last file at end of file
    off_t current_location = lseek(tar_fd, 0, SEEK_END);
    if (current_location < 0)
    {
        print_error("Unable to lseek file\n");
        return -1;
    }

    struct header *f_header;

    while (current_location >= 0) // this should take it all the way to 0 write to-> 512
    {
        my_memset(header_buffer, 0, sizeof(header_buffer));
        int n = 0;

        if (lseek(tar_fd, -512, SEEK_CUR) < 0)
        {
            print_error("Unable to lseek file\n");
            return -1;
        }
        n = read(tar_fd, header_buffer, 512);
        if ((n < 0) || n != 512)
        {
            print_error("Unable to read magic tar file\n");
            return -1;
        }

        f_header = (struct header *)header_buffer;

        // Extracting the entire tar file
        if ((f_header->magic[0] == 'u' &&
             f_header->magic[1] == 's' &&
             f_header->magic[2] == 't' &&
             f_header->magic[3] == 'a' &&
             f_header->magic[4] == 'r' &&
             f_header->magic[5] == ' '))
        {
            current_location = lseek(tar_fd, 0, SEEK_CUR);
            break;
        }

        current_location = lseek(tar_fd, -512, SEEK_CUR);
        if (current_location < 0)
        {
            print_error("Unable to lseek file\n");
            return -1;
        }
    }
    size_t file_size = parse_octal(f_header->size, sizeof(f_header->size));
    size_t num_blocks = file_size % BLOCKSIZE == 0 ? file_size / BLOCKSIZE : file_size / BLOCKSIZE + 1;
    current_location = lseek(tar_fd, current_location + num_blocks * BLOCKSIZE, SEEK_SET);
    off_t pre_process_location = current_location;

    for (int i = 0; i < num_archive_names; i++)
    {

        // if (process_entry(names[i], tar_fd) < 0 && (op_flag == 'r')) // temporary for 'r'
        current_location = process_entry(archive_names[i], tar_fd);
        if (current_location < pre_process_location && (op_flag == 'r')) // temporary for 'r'
        {
            file_error(archive_names[i]);
            prev_error_flag = 1;
        }
    }
    if (prev_error_flag == 1)
    {
        previous_errors();
    }

    off_t pre_extra_padding_location = lseek(tar_fd, current_location, SEEK_SET);

    current_location = add_zeros(tar_fd);
    if (current_location < pre_extra_padding_location)
    {
        print_error("Unable to add zero padding");
        return -1;
    }

    close(tar_fd);
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

    return names;
}

int open_file(char *file_name, int flags, int perms)
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

int open_tar(char **names) // int v_flag
{
    int tar_fd;

    char *tar_name = names[0];

    tar_fd = create_tar_file(tar_name, 't');
    return tar_fd;
}

// need to parse out the file names and use the same logic as found in create_tar just as open tar
int print_included_tar_contents(int tar_fd, char **names, int num_names)
{
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

    int *names_log = (int *)malloc(num_names * sizeof(int));
    if (!names_log)
    {
        failed_alloc();
        return -1;
    }
    my_memset(names_log, 0, num_names * sizeof(int));

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
        if ((f_header->magic[0] == 'u' &&
             f_header->magic[1] == 's' &&
             f_header->magic[2] == 't' &&
             f_header->magic[3] == 'a' &&
             f_header->magic[4] == 'r' &&
             f_header->magic[5] == ' '))
        {

            if (num_names == 1)
            {
                my_printf("%s\n", f_header->name);
            }

            else if (num_names > 1)
            {
                for (int i = 1; i < num_names; i++)
                {
                    if (my_strcmp(names[i], f_header->name) == 0)
                    {
                        my_printf("%s\n", names[i]);
                        names_log[i] = 1;
                    }
                }
            }
        }
    }

    if (num_names > 1)
    {
        int missing_names_flag = 0;
        for (int j = 1; j < num_names; j++)
        {
            if (names_log[j] == 0)
            {
                file_not_found_error(names[j]);
                missing_names_flag = 1;
            }
        }
        if (missing_names_flag)
        {
            previous_errors();
        }
    }

    free(names_log);
    names_log = NULL;

    close(tar_fd);
    return 0;
}

int create_tar_file(char *tar_name, char op_flag)
{
    int tar_fd;

    // create tar file:
    if (op_flag == 'c')
    {
        tar_fd = open_file(tar_name, O_RDWR | O_CREAT | O_TRUNC, TAR_PERMS);
    }

    else if (op_flag == 't')
    {
        tar_fd = open_file(tar_name, O_RDONLY, TAR_PERMS);
    }

    else if (op_flag == 'r' || op_flag == 'u')
    {
        tar_fd = open_file(tar_name, O_RDWR | O_CREAT, TAR_PERMS);
    }

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
    tar_fd = create_tar_file(tar_name, 'c');

    if (tar_fd < 0)
    {
        return -1;
    }

    int prev_error_flag = 0;

    for (int i = 1; i < num_names; i++)
    {
        if (process_entry(names[i], tar_fd) < 0)
        {
            my_printf("Error processing %s into tar file\n", names[i]);
            prev_error_flag = 1;
        }
    }

    if (prev_error_flag == 1)
    {
        previous_errors();
    }

    if (add_zeros(tar_fd) < 0)
    {
        print_error("Unable to add zero padding");
        return -1;
    }

    close(tar_fd);

    return 0;
}

off_t add_zeros(int tar_fd)
{
    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        return -1;
    }
    off_t current_location = lseek(tar_fd, 0, SEEK_CUR);

    // need two zero blocks and then need to see if that goes over the size of a record
    int zero_padding = 2 * BLOCKSIZE;
    int padded_data = current_location + zero_padding;

    int total_required_padding;

    int rec_num = (current_location % (RECORDSIZE * BLOCKSIZE) == 0) ? current_location / (RECORDSIZE * BLOCKSIZE) : current_location / (RECORDSIZE * BLOCKSIZE) + 1;

    int rec_num_wpad = ((padded_data) % (RECORDSIZE * BLOCKSIZE) == 0) ? padded_data / (RECORDSIZE * BLOCKSIZE) : padded_data / (RECORDSIZE * BLOCKSIZE) + 1;

    if (rec_num == rec_num_wpad)
    {
        total_required_padding = rec_num * (RECORDSIZE * BLOCKSIZE) - current_location;
    }
    else
    {
        total_required_padding = rec_num_wpad * (RECORDSIZE * BLOCKSIZE) - current_location;
    }

    off_t pre_pad_location = lseek(tar_fd, 0, SEEK_CUR);

    off_t post_pad_location = write_padding(tar_fd, total_required_padding);

    if (post_pad_location < pre_pad_location)
    {
        print_error("Unable to add padding\n");
        return -1;
    }
    current_location = lseek(tar_fd, 0, SEEK_CUR);

    return post_pad_location;
}

off_t process_entry(char *path, int tar_fd)
{
    off_t current_location = lseek(tar_fd, 0, SEEK_CUR);

    struct stat arg_stats;
    if (stat(path, &arg_stats) < 0)
    {
        file_error(path);
    }
    header *hdr = fill_header_info(path);
    if (!hdr)
    {
        file_error(path);
        return -1;
    }

    current_location = write_header(hdr, tar_fd);

    current_location = lseek(tar_fd, current_location, SEEK_SET);

    free(hdr);
    hdr = NULL;

    if (S_ISREG(arg_stats.st_mode))
    {
        // if file to append
        current_location = append_file_data(tar_fd, path);

        current_location = lseek(tar_fd, current_location, SEEK_SET);
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

            if (process_entry(rel_path, tar_fd) < 0)
            {
                print_error("Failure to process directory entries\n");
                closedir(dir);
                return -1;
            }
        }

        current_location = lseek(tar_fd, current_location, SEEK_SET);

        closedir(dir);
    }

    current_location = lseek(tar_fd, current_location, SEEK_SET);

    return current_location;
}

int extract_tar(char **names, int num_names) // int v_flag
{
    int tar_fd;

    char *tar_name = names[0];
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
        if ((n < 0) || n != 512)
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
        if (f_header->prefix[prefix_len - 1] != '/')
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
        int fd = open_file(file_name, file_flags, file_perms);
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
        if ((num < 0) || num != file_size)
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
        dir_name = NULL;
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

    struct timeval tv[2];
    time_t mtime = (time_t)parse_octal(f_header->mtime, sizeof(f_header->mtime));

    tv[0].tv_sec = mtime;
    tv[0].tv_usec = 0;
    tv[1].tv_sec = mtime;
    tv[1].tv_usec = 0;

    if (futimes(fd, tv) < 0)
    {
        print_error("Unable to set file times\n");
        return -1;
    }
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

off_t write_padding(int tar_fd, int total_required_padding)
{
    char zero_buff[total_required_padding];
    my_memset(zero_buff, 0, total_required_padding);

    ssize_t bytes_written = 0;

    unsigned char header_buffer[512];

    off_t current_location = lseek(tar_fd, 0, SEEK_END);
    if (current_location < 0)
    {
        print_error("Unable to lseek file\n");
        return -1;
    }

    struct header *f_header;

    while (current_location >= 0) // this should take it all the way to 0 write to-> 512
    {
        my_memset(header_buffer, 0, sizeof(header_buffer));
        int n = 0;

        if (lseek(tar_fd, -512, SEEK_CUR) < 0)
        {
            print_error("Unable to lseek file\n");
            return -1;
        }
        n = read(tar_fd, header_buffer, 512);
        if ((n < 0) || n != 512)
        {
            print_error("Unable to read magic tar file\n");
            return -1;
        }

        f_header = (struct header *)header_buffer;

        // Extracting the entire tar file
        if ((f_header->magic[0] == 'u' &&
             f_header->magic[1] == 's' &&
             f_header->magic[2] == 't' &&
             f_header->magic[3] == 'a' &&
             f_header->magic[4] == 'r' &&
             f_header->magic[5] == ' '))
        {
            current_location = lseek(tar_fd, 0, SEEK_CUR);

            int size = parse_octal(f_header->size, sizeof(f_header->size));

            if (size % BLOCKSIZE == 0)
            {
                lseek(tar_fd, current_location + size, SEEK_SET);
            }
            else
            {
                lseek(tar_fd, current_location + size + (BLOCKSIZE - size), SEEK_SET);
            }

            break;
        }
        current_location = lseek(tar_fd, -512, SEEK_CUR);
        if (current_location < 0)
        {
            print_error("Unable to lseek file\n");
            return -1;
        }
    }

    while (bytes_written < total_required_padding)
    {
        ssize_t written = write(tar_fd, zero_buff + bytes_written, total_required_padding - bytes_written);
        if (written < 0)
        {
            print_error("Failure to write data.\n");
            return -1;
        }
        bytes_written += written;
    }

    off_t end_location = lseek(tar_fd, 0, SEEK_CUR);

    return end_location;
}

off_t append_file_data(int tar_fd, char *append_file)
{
    off_t current_location = lseek(tar_fd, 0, SEEK_CUR);
    struct stat file_stats;

    if (stat(append_file, &file_stats) == -1)
    {
        return -1;
    }

    long int f_size = (long int)file_stats.st_size;

    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        return -1;
    }

    long int tar_size = (long int)tar_stats.st_size;

    current_location = lseek(tar_fd, 0, SEEK_CUR);

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

        current_location = lseek(tar_fd, 0, SEEK_CUR);

        if (ts_n < 0)
        {
            print_error("Unable to append file data\n");
            return -1;
        }
    }

    close(append_fd);
    return current_location; 
}

int write_header(header *hdr, int tar_fd)
{
    unsigned char *hdr_data = (unsigned char *)hdr;
    off_t current_location = lseek(tar_fd, 0, SEEK_CUR);

    ssize_t written = write(tar_fd, hdr_data, BLOCKSIZE);
    if (written < BLOCKSIZE)
    {
        print_error("write_header: write failed\n");
        return -1;
    }

    current_location += BLOCKSIZE;
    return current_location;
}

/*
write_file_data(): logic is for the partial writes in the events of system buffering and interrupts.
It a more common in pipes,fifos and sockets than regular files, but can possibly happen during larger
file writes.
*/
int write_file_data(int dst_fd, int src_fd, int f_size, int tar_flag)
{
    off_t start_offset = lseek(dst_fd, 0, SEEK_CUR);

    unsigned char transfer_buff[BLOCKSIZE];
    ssize_t total_bytes_written = 0;
    ssize_t additional_size = 0;
    ssize_t n = 0;
    ssize_t add_written = 0; // taken from the bottom conditional to carry through at end total

    while (1)
    {
        ssize_t bytes_to_read = BLOCKSIZE;

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

        int m = write(dst_fd, transfer_buff, n);
        if (m < n)
        {
            print_error("write error\n");
            return -1;
        }
        total_bytes_written += m;
    }
    // if writing to tar file add intra-block padding
    if (tar_flag == 1)
    {
        off_t abs_offset = start_offset = total_bytes_written;
        if (abs_offset % BLOCKSIZE != 0)
        {
            additional_size = BLOCKSIZE - (abs_offset % BLOCKSIZE);
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
    int write_size = (long int)total_bytes_written + (long int)add_written;
    return write_size;
}
