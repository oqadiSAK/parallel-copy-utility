#include "../include/producer.h"

void *producer_thread_function(void *args)
{
    producer_args_t *producer_args = (producer_args_t *)args;
    traverse_directory(producer_args->src_dir, producer_args->dest_dir);
    queue_set_done(buffer);
    return NULL;
}

void traverse_directory(const char *src_dir, const char *dest_dir)
{
    // Check if the source directory is a subdirectory of the destination directory
    if (strstr(dest_dir, src_dir) != NULL)
    {
        pthread_mutex_lock(&message_mutex);
        fprintf(stderr, "Cannot copy a directory, '%s', into itself or its subdirectory, '%s'\n", src_dir, dest_dir);
        pthread_mutex_unlock(&message_mutex);
        queue_set_error_flag(buffer);
        return;
    }

    if (queue_check_signal(buffer))
    {
        queue_set_done(buffer);
        return;
    }
    // Create the necessary subdirectories in the destination path
    struct stat st = {0};
    if (stat(dest_dir, &st) == -1)
    {
        // Destination directory doesn't exist, create it recursively
        if (mkdir(dest_dir, FILE_PRIVILEGES) == -1)
        {
            pthread_mutex_lock(&message_mutex);
            fprintf(stderr, "Failed to create destination directory: %s\n", dest_dir);
            perror("Error");
            printf("\n\n");
            pthread_mutex_unlock(&message_mutex);
        }
    }

    DIR *dir = opendir(src_dir);
    if (dir == NULL)
    {
        return;
    }
    queue_increment_fd_counter(buffer, 1);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (queue_check_signal(buffer))
        {
            break;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char src_path[MAX_PATH_LEN];
        snprintf(src_path, MAX_PATH_LEN, "%s/%s", src_dir, entry->d_name);

        char dest_path[MAX_PATH_LEN];
        snprintf(dest_path, MAX_PATH_LEN, "%s/%s", dest_dir, entry->d_name);

        struct stat file_info;
        if (stat(src_path, &file_info) == -1)
        {
            // Failed to get file information
            continue;
        }

        int dir_type = IFTODT(file_info.st_mode);
        if (dir_type == DT_DIR)
        {
            pthread_mutex_lock(&message_mutex);
            fprintf(stdout, "DIR copied from \"%s\" to \"%s\"\n", src_path, dest_path);
            pthread_mutex_unlock(&message_mutex);
            queue_set_statistic(buffer, DT_DIR);
            traverse_directory(src_path, dest_path);
        }
        else if (dir_type == DT_FIFO)
        {
            // Check if the FIFO already exists
            if (access(dest_path, F_OK) != 0)
            {
                // Create the FIFO in the destination path
                if (mkfifo(dest_path, file_info.st_mode) == -1)
                {
                    pthread_mutex_lock(&message_mutex);
                    fprintf(stderr, "Failed to create FIFO on producing: %s\n", dest_path);
                    perror("Error");
                    printf("\n\n");
                    pthread_mutex_unlock(&message_mutex);
                }
            }
            pthread_mutex_lock(&message_mutex);
            fprintf(stdout, "FIFO copied from \"%s\" to \"%s\"\n", src_path, dest_path);
            pthread_mutex_unlock(&message_mutex);
            queue_set_statistic(buffer, DT_FIFO);
        }
        else
        {
            int src_fd = open(src_path, O_RDONLY, FILE_PRIVILEGES);
            if (src_fd < 0)
            {
                pthread_mutex_lock(&message_mutex);
                fprintf(stderr, "Failed to open source file when producing: %s\n", src_path);
                perror("Error");
                printf("\n\n");
                pthread_mutex_unlock(&message_mutex);
                continue;
            }
            queue_increment_fd_counter(buffer, 1);
            // Open the destination file for writing (truncate if already exists)
            int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, FILE_PRIVILEGES);
            if (dest_fd < 0)
            {
                pthread_mutex_lock(&message_mutex);
                fprintf(stderr, "Failed to open destination file when producing: %s\n", dest_path);
                perror("Error");
                printf("\n\n");
                pthread_mutex_unlock(&message_mutex);
                close(src_fd);
                continue;
            }
            queue_increment_fd_counter(buffer, 1);

            // Create a file_data_t structure
            file_data_t *file_data = malloc(sizeof(file_data_t));
            file_data->d_type = dir_type;
            file_data->src_fd = src_fd;
            file_data->dest_fd = dest_fd;
            strncpy(file_data->src_filename, src_path, MAX_FILENAME_LEN);
            file_data->src_filename[MAX_FILENAME_LEN - 1] = '\0'; // Add null terminator
            strncpy(file_data->dest_filename, dest_path, MAX_FILENAME_LEN);
            file_data->dest_filename[MAX_FILENAME_LEN - 1] = '\0'; // Add null terminator

            // Push the file_data_t structure to the queue buffer
            queue_enqueue(buffer, file_data);
        }
    }

    closedir(dir);
    queue_decrement_fd_counter(buffer, 1);
}
