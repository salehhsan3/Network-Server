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

/* functions for initializing/handling threadinfo_t*/
threadinfo_t* createThreadInfo();
void initializeThreadInfo(threadinfo_t *tin);
void changeThreadAndTID(threadinfo_t *tin, pthread_t *thread, int tid);
void incrementStaticRequests(threadinfo_t *tin);
void incrementDynamicRequests(threadinfo_t *tin);
void incrementHandledRequests(threadinfo_t *tin);
void destroyThreadInfo(threadinfo_t *tin);

/* function for initializing/handling requests_t*/
void initFd(requests_t *request, int fd);
void updateToAvailable(requests_t *request);
void updateToUnAvailable(requests_t *request);
int isAvailable(requests_t *request);
int initArrivalTimeOfRequest(requests_t *request);
int initPickedTimeOfRequest(requests_t *request);
void calculateIntervalOfRequest(requests_t *request);

void requestHandle(int fd, threadinfo_t *tinfo, requests_t* req);

#endif
