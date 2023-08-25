#include "../include/helpers.h"

void check_usage(int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s <buffer_size> <num_consumers> <src_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    buffer_size = atoi(argv[1]);
    if (buffer_size <= 0)
    {
        fprintf(stderr, "Invalid buffer size: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    num_consumers = atoi(argv[2]);
    if (num_consumers <= 0)
    {
        fprintf(stderr, "Invalid number of consumers: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    strncpy(src_dir, argv[3], MAX_PATH_LEN);
    strncpy(dest_dir, argv[4], MAX_PATH_LEN);

    // Check if source directory exists
    DIR *dir = opendir(src_dir);
    if (dir == NULL)
    {
        perror("Failed to open source directory");
        exit(EXIT_FAILURE);
    }
    closedir(dir);

    // Check if destination directory exists
    dir = opendir(dest_dir);
    if (dir == NULL)
    {
        perror("Failed to open destination directory");
        exit(EXIT_FAILURE);
    }
    closedir(dir);
}

void print_elapsed_time()
{
    long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;
    printf("Elapsed time: %ld milliseconds\n", elapsed_time);
}

void print_statistics()
{
    int i;
    printf("Number of consumers: %d\n", num_consumers);
    printf("Buffer size: %d\n", buffer_size);
    printf("Total bytes copied: %ld\n", buffer->total_bytes_copied);
    for (i = 0; i < DT_WHT + 1; i++)
    {
        const char *dir_type_name = NULL;
        switch (buffer->dir_type_stats[i].dir_type)
        {
        case DT_UNKNOWN:
            dir_type_name = "Unknown";
            break;
        case DT_FIFO:
            dir_type_name = "FIFO";
            break;
        case DT_CHR:
            dir_type_name = "Character Device";
            break;
        case DT_DIR:
            dir_type_name = "Directory";
            break;
        case DT_BLK:
            dir_type_name = "Block Device";
            break;
        case DT_REG:
            dir_type_name = "Regular File";
            break;
        case DT_LNK:
            dir_type_name = "Symbolic Link";
            break;
        case DT_SOCK:
            dir_type_name = "Socket";
            break;
        case DT_WHT:
            dir_type_name = "Whiteout Entry";
            break;
        default:
            continue;
        }
        printf("%s copied: %d\n", dir_type_name, buffer->dir_type_stats[i].count);
    }
}

void print_exit_message()
{
    if (!buffer->error_flag)
    {
        printf("\n\n------------------------------------------------\n");
        if (buffer->signal_received)
            printf("\nThe program was interrupted by a %s signal, and the copying process was not completed in its entirety.", buffer->signal_str);
        else
            printf("\nCopying process completed successfully.");
        printf("\n\n------------------------------------------------\n");
        print_elapsed_time();
        print_statistics();
    }
}