#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <libgen.h>
#include "include/types.h"
#include "include/queue.h"
#include "include/producer.h"
#include "include/consumer.h"
#include "include/sig_handler.h"
#include "include/helpers.h"

void init();
void run();
void block_thread_signals();
void create_producer_thread();
void create_consumer_threads();
void wait_threads();
void clean_up();

int buffer_size, num_consumers, counter_consumers;
char src_dir[MAX_PATH_LEN], dest_dir[MAX_PATH_LEN];
queue_t *buffer;
pthread_mutex_t message_mutex;
pthread_t producer_thread, *consumer_threads, signal_thread;
struct timeval start_time, end_time;
sigset_t signal_set;
producer_args_t *producer_args;

int main(int argc, char *argv[])
{
    check_usage(argc, argv);
    init();
    run();
    print_exit_message();
    clean_up();
    return 0;
}

void init()
{
    counter_consumers = 0;
    consumer_threads = (pthread_t *)malloc(num_consumers * sizeof(pthread_t));
    if (consumer_threads == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for consumer threads\n");
        exit(EXIT_FAILURE);
    }
    buffer = malloc(sizeof(queue_t));
    queue_init(buffer, buffer_size);
    pthread_mutex_init(&message_mutex, NULL);
    block_thread_signals();
    pthread_create(&signal_thread, NULL, signal_handler_thread, &signal_set);
}

void run()
{
    create_producer_thread();
    gettimeofday(&start_time, NULL);
    create_consumer_threads();
    wait_threads();
    gettimeofday(&end_time, NULL);
}

void block_thread_signals()
{
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGSTOP);
    sigaddset(&signal_set, SIGTSTP);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGQUIT);
    sigaddset(&signal_set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
}

void create_producer_thread()
{
    producer_args = (producer_args_t *)malloc(sizeof(producer_args_t));
    // Check if the destination directory exists within the source directory
    char dest_path[(MAX_PATH_LEN * 2) + 1];
    snprintf(dest_path, MAX_PATH_LEN * 2, "%s/%s", dest_dir, basename(src_dir));
    DIR *dir = opendir(dest_path);
    if (dir == NULL)
    {
        // Destination directory doesn't exist, create it
        if (mkdir(dest_path, FILE_PRIVILEGES) != 0)
        {
            fprintf(stderr, "Failed to create destination directory: %s\n", dest_path);
            perror("Error");
            printf("\n\n");
            exit(EXIT_FAILURE);
        }
    }
    closedir(dir);

    strncpy(producer_args->src_dir, src_dir, MAX_PATH_LEN);
    strncpy(producer_args->dest_dir, dest_path, MAX_PATH_LEN);
    pthread_create(&producer_thread, NULL, producer_thread_function, producer_args);
}

void create_consumer_threads()
{
    int i;
    for (i = 0; i < num_consumers; i++)
    {
        if (queue_check_signal(buffer))
            break;
        pthread_create(&consumer_threads[i], NULL, consumer_thread_function, NULL);
        counter_consumers++;
    }
}

void wait_threads()
{
    pthread_join(producer_thread, NULL);
    int i;
    for (i = 0; i < counter_consumers; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }
}

void clean_up()
{
    pthread_kill(signal_thread, SIGUSR1);
    pthread_join(signal_thread, NULL);
    free(producer_args);
    queue_destroy(buffer);
    free(consumer_threads);
}
