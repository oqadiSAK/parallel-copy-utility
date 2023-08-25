#include "../include/queue.h"

void queue_init(queue_t *queue, int capacity)
{
    int i;
    pthread_cond_init(&(queue->enqueue_cv), NULL);
    pthread_cond_init(&(queue->dequeue_cv), NULL);
    pthread_mutex_init(&(queue->lock), NULL);
    queue->data = (file_data_t **)malloc(sizeof(file_data_t *) * capacity);
    for (i = 0; i < capacity; ++i)
    {
        queue->data[i] = NULL;
    }
    queue->front = 0;
    queue->rear = 0;
    queue->capacity = capacity;
    queue->size = 0;
    queue->done_flag = 0;
    queue->total_bytes_copied = 0;
    for (i = 0; i < DT_WHT + 1; i++)
    {
        queue->dir_type_stats[i].dir_type = i;
        queue->dir_type_stats[i].count = 0;
    }
    queue->signal_received = 0;
    queue->signal_str = NULL;
    queue->error_flag = 0;
    queue->fd_counter = 0;
}

void queue_destroy(queue_t *queue)
{
    pthread_cond_destroy(&(queue->enqueue_cv));
    pthread_cond_destroy(&(queue->dequeue_cv));
    pthread_mutex_destroy(&(queue->lock));

    if (queue->signal_received)
    {
        int i;
        for (i = 0; i < queue->capacity; i++)
        {
            if (queue->data[i] != NULL)
                free(queue->data[i]);
        }
    }
    free(queue->data);
    free(queue);
}

void queue_enqueue(queue_t *queue, file_data_t *item)
{
    pthread_mutex_lock(&(queue->lock));
    while (((queue->size == queue->capacity) || (queue->fd_counter >= MAX_FD_NUMBER - 1)) && (!queue->signal_received))
        pthread_cond_wait(&(queue->enqueue_cv), &(queue->lock));

    if (!queue->signal_received)
    {
        queue->data[queue->rear] = item;
        queue->rear = (queue->rear + 1) % queue->capacity;
        queue->size += 1;
    }
    else
    {
        free(item);
    }
    pthread_cond_signal(&(queue->dequeue_cv));
    pthread_mutex_unlock(&(queue->lock));
}

file_data_t *queue_dequeue(queue_t *queue)
{
    pthread_mutex_lock(&(queue->lock));
    while ((queue->size == 0 && !queue->done_flag) && !queue->signal_received)
        pthread_cond_wait(&(queue->dequeue_cv), &(queue->lock));

    if (queue->size == 0 && queue->done_flag)
    {
        pthread_mutex_unlock(&(queue->lock));
        return NULL;
    }

    file_data_t *retval = queue->data[queue->front];
    queue->data[queue->front] = NULL;
    if (queue->signal_received)
    {
        pthread_cond_signal(&(queue->enqueue_cv));
        pthread_mutex_unlock(&(queue->lock));
        return NULL;
    }
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size -= 1;

    pthread_cond_signal(&(queue->enqueue_cv));
    pthread_mutex_unlock(&(queue->lock));
    return retval;
}

void queue_set_done(queue_t *queue)
{
    pthread_mutex_lock(&(queue->lock));
    queue->done_flag = 1;
    pthread_cond_broadcast(&(queue->dequeue_cv));
    pthread_mutex_unlock(&(queue->lock));
}

void queue_set_statistic(queue_t *queue, int d_type)
{
    pthread_mutex_lock(&(queue->lock));
    queue->dir_type_stats[d_type].count++;
    pthread_mutex_unlock(&(queue->lock));
}

void queue_set_error_flag(queue_t *queue)
{
    pthread_mutex_lock(&(queue->lock));
    queue->error_flag = 1;
    pthread_mutex_unlock(&(queue->lock));
}

void queue_increment_bytes_copied(queue_t *queue, int bytes)
{
    pthread_mutex_lock(&(queue->lock));
    queue->total_bytes_copied += bytes;
    pthread_mutex_unlock(&(queue->lock));
}

void queue_set_signal(queue_t *queue, char *signal_str)
{
    pthread_mutex_lock(&(queue->lock));
    queue->signal_received = 1;
    queue->signal_str = signal_str;
    pthread_cond_broadcast(&(queue->enqueue_cv));
    pthread_cond_broadcast(&(queue->dequeue_cv));
    pthread_mutex_unlock(&(queue->lock));
}

int queue_check_signal(queue_t *queue)
{
    int retval;
    pthread_mutex_lock(&(queue->lock));
    retval = queue->signal_received;
    pthread_mutex_unlock(&(queue->lock));
    return retval;
}

void queue_increment_fd_counter(queue_t *queue, int number)
{
    pthread_mutex_lock(&(queue->lock));
    queue->fd_counter += number;
    pthread_mutex_unlock(&(queue->lock));
}

void queue_decrement_fd_counter(queue_t *queue, int number)
{
    pthread_mutex_lock(&(queue->lock));
    queue->fd_counter -= number;
    pthread_mutex_unlock(&(queue->lock));
}