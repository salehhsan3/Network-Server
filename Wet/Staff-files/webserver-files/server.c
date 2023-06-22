#include "segel.h"
#include "request.h"
#include "queue.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
#define SCHED_BLOCK 1
#define SCHED_DT 2
#define SCHED_DH 3
#define SCHED_BF 4
#define SCHED_DYNAMIC 5
#define SCHED_RAND 6

Queue wait_q; //has to be global because it will be accessed by all threads in our system
Queue worker_q; //has to be global because it will be accessed by all threads in our system
extern pthread_mutex_t m_lock;
extern pthread_cond_t block_cond;
extern pthread_cond_t block_cond_main;
pthread_t* thread_arr = NULL;
threadinfo_t* tinfo_arr = NULL;
requests_t* requests_arr_per_thread = NULL;
static const char sched_block[6]       = "block";
static const char sched_dt[3]          = "dt";
static const char sched_dh[3]          = "dh";
static const char sched_bf[3]          = "bf";
static const char sched_dynamic[8]     = "dynamic";
static const char sched_random[7]      = "random";

int findScheduler(char *sched_alg)
{
    if (strcmp(sched_alg,sched_block) == 0)
    {
        return SCHED_BLOCK;
    }
    else if (strcmp(sched_alg,sched_dh) == 0)
    {
        return SCHED_DH;
    }
    else if (strcmp(sched_alg,sched_dt) == 0)
    {
        return SCHED_DT;
    }
    else if (strcmp(sched_alg,sched_bf) == 0)
    {
        return SCHED_BF;
    }
    else if (strcmp(sched_alg,sched_dynamic) == 0)
    {
        return SCHED_DYNAMIC;
    }
    else if (strcmp(sched_alg,sched_random) == 0)
    {
        return SCHED_RAND;
    }

    return 0;
}

void getargs(int *port, int *thread_num, int *queue_size, char *sched_alg,
             int* max_size, int* algorithm, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(sched_alg,argv[4]);
    *algorithm = findScheduler(sched_alg);
    if ( (*algorithm) == SCHED_DYNAMIC )
    {
        (*max_size) = atoi(argv[5]);
    }
}

void* start_routine(void *arguments)
{
    while (1)
    {
        pthread_mutex_lock(&m_lock);

        while ( currentSizeOfQueue(wait_q) <= 0 )
        {
            pthread_cond_wait( &block_cond, &m_lock );
        }

        int t_inc_index = ((int*)arguments)[0];
        Node to_add = findNode(wait_q, 1); 
        requests_t* req = getData(to_add);
        initPickedTimeOfRequest(req);    
        calculateIntervalOfRequest(req); 
        memcpy(requests_arr_per_thread + t_inc_index, req, sizeof(requests_t) );
        req = (requests_t*) (requests_arr_per_thread + t_inc_index);

        pop(wait_q);
        push(worker_q, req);

        pthread_mutex_unlock( &m_lock );

        requestHandle(req->fd, (threadinfo_t*)(tinfo_arr + t_inc_index), req);

        pthread_mutex_lock(&m_lock);

        t_inc_index = ((int*)arguments)[0];
        req = (requests_t*)(requests_arr_per_thread + t_inc_index);
        int request_fd = req->fd;
        popRequest(worker_q, req);
        Close(request_fd);

        pthread_cond_signal( &block_cond );
        pthread_cond_signal( &block_cond_main );
        pthread_mutex_unlock(&m_lock);

    }
    return NULL;
}

void createWorkerThreads(int thread_num, int *thread_args)
{
    for (int i = 0; i < thread_num; i++)
    {
        thread_args[i] = i;
        pthread_create(&thread_arr[i],NULL,(void*)start_routine, &thread_args[i]);
    }

    tinfo_arr = (threadinfo_t*)malloc(sizeof(threadinfo_t) * thread_num);

    
    for (int i = 0; i < thread_num; i++)
    {
        initializeInfo( (threadinfo_t*)(tinfo_arr + i));
        changeThreadAndTheadId(   (threadinfo_t*)(tinfo_arr + i), (pthread_t*)(thread_arr + i), i);
    }
}

void releaseResources(Queue wait_q, Queue worker_q, pthread_t* thread_arr, int* thread_args, threadinfo_t* tinfo_arr, requests_t* req_arr, requests_t* requests_arr_per_thread)
{
    destroyQueue(worker_q);
    destroyQueue(wait_q);
    free(thread_arr);
    free(thread_args);
    free(tinfo_arr);
    free(req_arr);
    free(requests_arr_per_thread);
}

int findIndexOfFirstAvailableRequest(requests_t* req_arr, int arr_size)
{
    for (int i = 0; i < arr_size; i++)
    {
        if (isAvailable(&req_arr[i]))
        {
            return i;
        }  
    }
    return 0;
}

void initializeLocksAndConditions(pthread_mutex_t* m_lock, pthread_cond_t* block_cond, pthread_cond_t* block_cond_main)
{
    pthread_mutex_init(m_lock, NULL);    
    pthread_cond_init(block_cond, NULL); 
    pthread_cond_init(block_cond_main, NULL);
}

int main(int argc, char *argv[])
{
    initializeLocksAndConditions(&m_lock, &block_cond, &block_cond_main);

    int listenfd, connfd, port, clientlen, queue_size, thread_num, algorithm;
    int max_size = 0;
    char sched_alg[7];
    struct sockaddr_in clientaddr;
    getargs(&port,&thread_num,&queue_size,sched_alg, &max_size, &algorithm, argc, argv);
    int req_arr_size = (max_size >= queue_size) ? max_size : queue_size;
    int isExpandable = (algorithm == SCHED_DYNAMIC);
    wait_q   = createQueue(queue_size, max_size, isExpandable);
    worker_q = createQueue(queue_size, max_size, isExpandable);
    if (wait_q == NULL || worker_q == NULL)
    {
       return 0;
    }
    int *thread_args = (int*)malloc(sizeof(int) * thread_num);
    requests_t* req_arr = (requests_t*) malloc(sizeof(*req_arr) * req_arr_size);
    for (int i = 0; i < req_arr_size; i++)
    {
        updateToAvailable(&req_arr[i]);
    }
    requests_arr_per_thread = (requests_t*) malloc(sizeof(requests_t) * thread_num); 
    thread_arr = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);
    createWorkerThreads(thread_num, thread_args);
    listenfd = Open_listenfd(port);
    int insert_ind = 0;
    while (1) {
START_OF_WHILE:
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    pthread_mutex_lock(&m_lock);

    insert_ind = findIndexOfFirstAvailableRequest(req_arr, req_arr_size);
    updateFd( &(req_arr[insert_ind]), connfd );
    initArrivalTimeOfRequest( &(req_arr[insert_ind]));

    if ( (currentSizeOfQueue(worker_q) + currentSizeOfQueue(wait_q)) == capacityOfQueue(worker_q)  ) 
    {
        switch (algorithm)
        {

            case SCHED_BLOCK:

                while ( (currentSizeOfQueue(worker_q) + currentSizeOfQueue(wait_q)) == capacityOfQueue(worker_q)  ) 
                {
                    pthread_cond_signal( &block_cond );
                    pthread_cond_wait(&block_cond_main, &m_lock); 
                }

                break;

            case SCHED_DT:
            DROP_TAIL_POLICY:
                Close(connfd);
                pthread_mutex_unlock(&m_lock);
                goto START_OF_WHILE;

                break;

            case SCHED_BF:
                    /*-------------------------------------------------------------------------*/
                    while ( currentSizeOfQueue(wait_q) + currentSizeOfQueue(worker_q) > 0  ) 
                    {
                        // pthread_cond_signal( &block_cond );
                        pthread_cond_wait(&block_cond_main, &m_lock); // block incoming requests   
                    }
                    Close(connfd); // drop request after dropping all requests!
                    pthread_mutex_unlock(&m_lock);
                    continue;
                break;

            case SCHED_DH:
                //check if wait_q is empty or not
                if (isQueueEmpty(wait_q) == TRUE)
                {
                    //this means: worker_q is full
                    Close(connfd);
                    pthread_mutex_unlock(&m_lock);
                    continue; 
                }
                
                else //find oldest request in wait_q
                {
                    Node first_node = findNode(wait_q,1);
                    requests_t *first_data = getData(first_node);
                    Close(first_data->fd);
                    popRequest(wait_q, first_data);
                }

                break;

            case SCHED_RAND:
                
                //check if queue is empty or not
                if (isQueueEmpty(wait_q))
                {
                    // nothing to drop here
                    Close(connfd);
                    pthread_mutex_unlock(&m_lock);
                    continue; 
                }
                else
                {
                    //drop half of the requests in the queue
                    int q_size = currentSizeOfQueue(wait_q);
                    int to_remove = (int) ((q_size + 1) / 2); 
                    while ( to_remove > 0)
                    {
                        to_remove --;
                        popRandom(wait_q);
                    }
                }

                break;
            
            case SCHED_DYNAMIC:
                /*-------------------------------------------------------------------------*/
                while ( currentSizeOfQueue(wait_q) + currentSizeOfQueue(worker_q) ==  capacityOfQueue(wait_q)  ) 
                {
                    if ( !isQueueExpandable(wait_q) )
                    {
                        goto DROP_TAIL_POLICY;
                    }
                    else
                    {
                        Close(connfd);
                        ExpandQueue(wait_q);
                        ExpandQueue(worker_q);
                        pthread_mutex_unlock(&m_lock);
                        goto START_OF_WHILE;    
                    }
                }
                break;

        default:
            exit(0);
            break;
        }

    }

    push(wait_q, &req_arr[insert_ind]);

    pthread_cond_signal( &block_cond );
    pthread_mutex_unlock(&m_lock);
    }

    releaseResources(wait_q, worker_q, thread_arr, thread_args, tinfo_arr, req_arr, requests_arr_per_thread);
}
