#ifndef PRODUCER_H
#define PRODUCER_H

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "types.h"
#include "queue.h"

extern queue_t *buffer;
extern int buffer_size;
extern pthread_mutex_t message_mutex;

void *producer_thread_function(void *args);
void traverse_directory(const char *src_dir, const char *dest_dir);

#endif
