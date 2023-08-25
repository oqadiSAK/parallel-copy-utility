#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"

void queue_init(queue_t *queue, int capacity);
void queue_destroy(queue_t *queue);
void queue_enqueue(queue_t *queue, file_data_t *item);
file_data_t *queue_dequeue(queue_t *queue);
void queue_set_done(queue_t *queue);
void queue_set_statistic(queue_t *queue, int d_type);
void queue_set_error_flag(queue_t *queue);
void queue_increment_bytes_copied(queue_t *queue, int bytes);
void queue_set_signal(queue_t *queue, char *signal_str);
int queue_check_signal(queue_t *queue);
void queue_increment_fd_counter(queue_t *queue, int number);
void queue_decrement_fd_counter(queue_t *queue, int number);

#endif
