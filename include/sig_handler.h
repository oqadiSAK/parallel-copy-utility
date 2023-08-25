#ifndef SIG_HANDLER_H
#define SIG_HANDLER_H

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "queue.h"

extern queue_t *buffer;
extern pthread_mutex_t message_mutex;

void *signal_handler_thread(void *arg);

#endif // SIG_HANDLER_H
