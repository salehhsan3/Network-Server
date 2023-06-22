#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    int fd;                             /* descriptor for handling the request*/
    int available;                      /*a boolean value to denote whether a request is available or not*/
    struct timeval arrival_time;        /* indicates the arrival time of this specific request*/
    struct timeval picked_time;         /* indicates the picked up time of this specific request*/
    struct timeval dispatch_time;            /* indicates the dispatched time of this specific request*/
} requests_t;



/* Additions for a comfortable and clear implementation regarding Threads and their statistics*/
typedef struct {
    pthread_t *thread;                  /* the actual thread stored */
    int thread_id;                    /* id for this specific thread */
    int static_requests_num;         /* number of static requests handles by this thread */
    int dynamic_requests_num;       /* number of static requests handles by this thread */
    int handled_requests_num;       /* number of total requests handles by this thread */
} threadinfo_t;

/* functions to interact with threadinfo_t*/
void initializeInfo(threadinfo_t *t_info);
void changeThreadAndTheadId(threadinfo_t *t_info, pthread_t *thread, int thread_id);
void incrementHandledDynamicRequests(threadinfo_t *t_info);
void incrementHandledStaticRequests(threadinfo_t *t_info);
void incrementTotalHandledRequests(threadinfo_t *t_info);

/* function to interact with requests_t*/
void updateFd(requests_t *req, int fd);
void updateToAvailable(requests_t *req);
void updateToUnAvailable(requests_t *req);
int isAvailable(requests_t *req);
int initArrivalTimeOfRequest(requests_t *req);
int initPickedTimeOfRequest(requests_t *req);
void calculateIntervalOfRequest(requests_t *req);

void requestHandle(int fd, threadinfo_t *t_info, requests_t* req);

#endif
