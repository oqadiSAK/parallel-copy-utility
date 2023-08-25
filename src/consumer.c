#include "../include/consumer.h"

void *consumer_thread_function()
{
    while (1)
    {
        if (queue_check_signal(buffer))
            return NULL;

        file_data_t *file_data = queue_dequeue(buffer);
        if (file_data == NULL)
        {
            return NULL;
        }
        copy_file(file_data);
        free(file_data);
    }

    return NULL;
}

void copy_file(file_data_t *file_data)
{
    ssize_t bytes_read = 0, bytes_written = 0;
    char read_buffer[256];
    while ((bytes_read = read(file_data->src_fd, read_buffer, sizeof(read_buffer))) > 0)
    {
        if (queue_check_signal(buffer))
        {
            close(file_data->src_fd);
            close(file_data->dest_fd);
            queue_decrement_fd_counter(buffer, 2);
            return;
        }
        bytes_written = write(file_data->dest_fd, read_buffer, bytes_read);
        if (bytes_written < 0)
        {
            pthread_mutex_lock(&message_mutex);
            fprintf(stderr, "Failed to write to destination file when consuming: %s\n", file_data->dest_filename);
            fprintf(stderr, "\n\nFd : %d\n\n", file_data->dest_fd);
            perror("Error");
            printf("\n\n");
            pthread_mutex_unlock(&message_mutex);
            break;
        }
        else
        {
            queue_increment_bytes_copied(buffer, bytes_written);
        }
    }

    close(file_data->src_fd);
    close(file_data->dest_fd);
    queue_decrement_fd_counter(buffer, 2);

    if (bytes_written < 0)
    {
        return;
    }

    if (bytes_read < 0)
    {
        pthread_mutex_lock(&message_mutex);
        fprintf(stderr, "Failed to read from source file when consuming: %s\n", file_data->src_filename);
        fprintf(stderr, "\n\nFd : %d\n\n", file_data->src_fd);
        perror("Error");
        printf("\n\n");
        pthread_mutex_unlock(&message_mutex);
        return;
    }

    pthread_mutex_lock(&message_mutex);
    fprintf(stdout, "File copied from \"%s\" to \"%s\"\n", file_data->src_filename, file_data->dest_filename);
    pthread_mutex_unlock(&message_mutex);

    queue_set_statistic(buffer, file_data->d_type);
}
