#ifndef CONSUMER_H
#define CONSUMER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "types.h"
#include "queue.h"

extern queue_t *buffer;
extern int buffer_size;
extern pthread_mutex_t message_mutex;

void *consumer_thread_function();
void copy_file(file_data_t *file_data);

#endif
