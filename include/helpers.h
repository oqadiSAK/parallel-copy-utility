#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "queue.h"
#include "types.h"

extern queue_t *buffer;
extern struct timeval start_time, end_time;
extern int buffer_size, num_consumers;
extern char src_dir[MAX_PATH_LEN], dest_dir[MAX_PATH_LEN];

void check_usage(int argc, char *argv[]);
void print_elapsed_time();
void print_statistics();
void print_exit_message();

#endif // HELPERS_H
