int check_newest_names(int tar_fd, Hashtable *table)
{
    if (!table || !table->buckets || tar_fd < 0)
    { // Added NULL checks
        print_error("Invalid arguments\n");
        return -1;
    }

    int num_vetted_names = table->num_vetted_names;
    int num_newest_names = num_vetted_names;
    struct stat tar_stats;

    if (fstat(tar_fd, &tar_stats) == -1)
    {
        print_error("Unable to stat tar\n");
        return -1;
    }

    off_t tar_size = tar_stats.st_size; // Use proper type for file size

    if (lseek(tar_fd, 0, SEEK_SET) < 0)
    {
        print_error("Unable to lseek file\n");
        return -1;
    }

    unsigned char header_buffer[512];
    ssize_t read_size = 0; // Use proper type for read operations

    while (read_size < tar_size)
    {
        memset(header_buffer, 0, sizeof(header_buffer));

        // Read entire header block
        ssize_t n = read(tar_fd, header_buffer, 512);
        if (n < 0 || n != 512)
        { // Fixed read validation
            print_error("Invalid tar block read\n");
            return -1;
        }
        read_size += n;

        struct header *f_header = (struct header *)header_buffer;

        // Validate UStar magic (critical fix)
        if (memcmp(f_header->magic, "ustar", 5) != 0 || f_header->magic[5] != '\0')
        {
            // Skip non-ustar headers
            continue;
        }

        // Ensure name is null-terminated for safety
        char filename[NAMESIZE + 1] = {0};
        strncpy(filename, f_header->name, NAMESIZE);

        // Calculate safe hash index (critical fix)
        int f_hash = hash_fn(filename, table->num_buckets) % table->num_buckets;

        if (num_vetted_names == 0)
        {
            printf("my_tar command needs additional arguments\n");
            return -1;
        }

        // Check bucket existence
        if (!table->buckets[f_hash])
        {
            printf("No entry for %s\n", filename);
            continue; // Changed from fatal error to continue
        }

        File_Entry *iterator = table->buckets[f_hash];
        while (iterator)
        {
            // Safe string comparison (critical fix)
            if (strncmp(iterator->name, filename, NAMESIZE) == 0)
            {
                if (iterator->newest_version_flag)
                {
                    time_t f_mtime = parse_octal(f_header->mtime, sizeof(f_header->mtime));

                    // Debug prints (recommended)
                    printf("Checking %s: stored_mtime=%ld, tar_mtime=%ld\n",
                           filename, iterator->mod_time, f_mtime);

                    if (iterator->mod_time <= f_mtime)
                    {
                        iterator->newest_version_flag = 0;
                        num_newest_names--;
                    }
                }
                break; // Found matching entry
            }
            iterator = iterator->next;
        }
    }
    return 0;
}