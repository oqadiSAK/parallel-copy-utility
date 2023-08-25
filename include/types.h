#ifndef TYPES_H
#define TYPES_H

#define MAX_PATH_LEN 4096
#define MAX_FILENAME_LEN 255
#define MAX_FD_NUMBER 1000
#define FILE_PRIVILEGES 077

#include <pthread.h>
#include <dirent.h>
#include <signal.h>

typedef struct
{
    char src_dir[MAX_PATH_LEN];
    char dest_dir[MAX_PATH_LEN];
} producer_args_t;

typedef struct
{
    int src_fd;
    int dest_fd;
    char src_filename[MAX_FILENAME_LEN];
    char dest_filename[MAX_FILENAME_LEN];
    int d_type; // this stands for directory type
} file_data_t;

typedef struct
{
    int dir_type;
    int count;
} statistics_t;

typedef struct
{
    pthread_cond_t enqueue_cv;
    pthread_cond_t dequeue_cv;
    pthread_mutex_t lock;

    file_data_t **data;
    int front;
    int rear;
    int capacity;
    int size;
    int done_flag;
    int error_flag;
    int fd_counter;
    unsigned long int total_bytes_copied;
    statistics_t dir_type_stats[DT_WHT + 1];

    volatile sig_atomic_t signal_received;
    char *signal_str;
} queue_t;

#endif