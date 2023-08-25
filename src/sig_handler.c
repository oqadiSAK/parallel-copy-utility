#include "../include/sig_handler.h"

void *signal_handler_thread(void *arg)
{
    sigset_t *signal_set = (sigset_t *)arg;
    int signo;
    siginfo_t info;
    char *signal_str = NULL;
    if (sigwaitinfo(signal_set, &info) == -1)
    {
        perror("sigwaitinfo");
        return NULL;
    }
    signo = info.si_signo;

    if (signo == SIGUSR1)
    {
        return NULL;
    }
    else if (signo == SIGINT)
        signal_str = "SIGINT";
    else if (signo == SIGSTOP)
        signal_str = "SIGSTOP";
    else if (signo == SIGTSTP)
        signal_str = "SIGTSTP";
    else if (signo == SIGTERM)
        signal_str = "SIGTERM";
    else if (signo == SIGQUIT)
        signal_str = "SIGQUIT";

    if (signal_str != NULL)
    {
        pthread_mutex_lock(&message_mutex);
        printf("\n\nReceived %s signal. Closing the threads...\n\n", signal_str);
        pthread_mutex_unlock(&message_mutex);
        queue_set_signal(buffer, signal_str);
    }

    return NULL;
}