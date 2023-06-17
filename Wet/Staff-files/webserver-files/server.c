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

int pickSchedulingAlgorithm(char *sched_alg)
{
    if (strcmp(sched_alg,sched_block) == 0)
    {
        return SCHED_BLOCK;
    }
    else if (strcmp(sched_alg,sched_dt) == 0)
    {
        return SCHED_DT;
    }
    else if (strcmp(sched_alg,sched_dh) == 0)
    {
        return SCHED_DH;
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
             int* max_size, int* alg_option, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(sched_alg,argv[4]);
    *alg_option = pickSchedulingAlgorithm(sched_alg);
    if ( (*alg_option) == SCHED_DYNAMIC )
    {
        (*max_size) = atoi(argv[5]);
    }
}

void* thread_start(void *arguments)
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
        memcpy(requests_arr_per_thread + t_inc_index, req, sizeof(requests_t) ); //copy to the THREADS array.
        req = (requests_t*) (requests_arr_per_thread + t_inc_index);

        dequeue(wait_q);
        int to_add_id = currentSizeOfQueue(worker_q) + 1;
        enqueue(worker_q, req, to_add_id);

        pthread_mutex_unlock( &m_lock );
        requestHandle(req->fd, (threadinfo_t*) (tinfo_arr + t_inc_index), req);
        pthread_mutex_lock(&m_lock);

        t_inc_index = ((int*)arguments)[0];
        req = (requests_t*) (requests_arr_per_thread + t_inc_index);
        int saveFD = req->fd;
        dequeueElementByRequest(worker_q, req);
        Close(saveFD);

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
        pthread_create(&thread_arr[i],NULL,(void*)thread_start, &thread_args[i]);
    }

    tinfo_arr = (threadinfo_t*)malloc(sizeof(threadinfo_t) * thread_num);

    
    for (int i = 0; i < thread_num; i++)
    {
        initializeThreadInfo( (threadinfo_t*)(tinfo_arr + i));
        changeThreadAndTID(   (threadinfo_t*)(tinfo_arr + i), (pthread_t*)(thread_arr + i), i);
    }
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&m_lock, NULL);    
    pthread_cond_init( &block_cond, NULL); 
    pthread_cond_init( &block_cond_main, NULL);

    int listenfd, connfd, port, clientlen, queue_size, thread_num, alg_option;
    int max_size = 0;
    char sched_alg[7];
    struct sockaddr_in clientaddr;
    getargs(&port,&thread_num,&queue_size,sched_alg, &max_size, &alg_option, argc, argv);
    int req_arr_size = (max_size >= queue_size) ? max_size : queue_size;

    wait_q   = createQueue(queue_size, max_size);
    worker_q = createQueue(queue_size, max_size);
    if (wait_q == NULL || worker_q == NULL)
    {
       return 0;
    }

    int *thread_args = (int*)malloc(sizeof(int) * thread_num);
    requests_t* req_arr = (requests_t*) malloc(sizeof(*req_arr) * req_arr_size);
    requests_arr_per_thread = (requests_t*) malloc(sizeof(requests_t) * thread_num); //array of pointers to requests_t
    thread_arr = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);

    

    createWorkerThreads(thread_num, thread_args);
    listenfd = Open_listenfd(port);
    int c = 0;
    int insert_ind = 0;
    int insert_ind_before_mod = 0;
    int expanded_queue = 0;
    while (1) {
START_OF_WHILE:
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    pthread_mutex_lock(&m_lock);
    insert_ind = c % capacityofQueue(wait_q);
    if (insert_ind != insert_ind_before_mod && expanded_queue) //to review!
    { 
        // this case is relevant when insert_ind_before_mod == 0 and insert_ind != 0
        // this could happen due to the changes in queue_size.
        // in all other cases insert_ind_before_mod == insert_ind
        initFd( &(req_arr[insert_ind_before_mod]), connfd );
        initArrivalTimeOfRequest( &(req_arr[insert_ind_before_mod]));
        expanded_queue = 0;
    }
    else
    {
        initFd( &(req_arr[insert_ind]), connfd );
        initArrivalTimeOfRequest( &(req_arr[insert_ind]));
    }
    initFd( &(req_arr[insert_ind]), connfd );
    initArrivalTimeOfRequest( &(req_arr[insert_ind]));


    if ( (currentSizeOfQueue(worker_q) + currentSizeOfQueue(wait_q)) == capacityofQueue(worker_q)  ) 
    {

        // handle according to which sched_alg was given in the input!
        switch (alg_option)
        {

            case SCHED_BLOCK:

                while ( (currentSizeOfQueue(worker_q) + currentSizeOfQueue(wait_q)) == capacityofQueue(worker_q)  ) 
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
                    dequeueElementByRequest(wait_q, first_data);
                }

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

            case SCHED_DYNAMIC:
                /*-------------------------------------------------------------------------*/
                while ( currentSizeOfQueue(wait_q) + currentSizeOfQueue(worker_q) ==  capacityofQueue(wait_q)  ) 
                {
                    if (!isQueueExpandable(wait_q))
                    {
                        goto DROP_TAIL_POLICY;
                    }
                    // else:
                    Close(connfd);
                    insert_ind_before_mod = c % capacityofQueue(wait_q);
                    ExpandQueue(wait_q);
                    ExpandQueue(worker_q);
                    expanded_queue = 1;
                    pthread_mutex_unlock(&m_lock);
                    goto START_OF_WHILE;    
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
                        int random_id = (rand() % q_size) + 1;                        
                        q_size --;
                        to_remove --;

                        Node node_to_rem = findNode(wait_q,random_id);
                        requests_t* req_to_rem = getData(node_to_rem);
                        resetIndices(wait_q);
                        Close(req_to_rem->fd);
                        dequeueElementByRequest(wait_q, req_to_rem);
                        resetIndices(wait_q);
                    }
                }

                break;

        default:
            exit(0); // if the scheduling algorithm is none of the options above then there's an error!
            break;
        }

    }



    int node_id_insert = currentSizeOfQueue(wait_q)+1;
    enqueue(wait_q, &req_arr[c % capacityofQueue(worker_q)], node_id_insert);
    c++;

    pthread_cond_signal( &block_cond );
    pthread_mutex_unlock(&m_lock);
    // pthread_cond_signal( &block_cond );
    }


    destroyQueue(worker_q);
    destroyQueue(wait_q);
    free(thread_arr);
    free(thread_args);
    free(tinfo_arr);
    free(req_arr);
    free(requests_arr_per_thread);
}


    


 
/* saving my work:

workerthreads:

// pthread_t *thread_arr = (pthread_t*)malloc(sizeof(*thread_arr)*thread_num);
// for (int i = 0; i < thread_num; i++)
// {
//     // a tip by Mousa: do the mutex implementation inside of the Queue implementation so that I can test the correctness of modules as seperate parts without testing the overall homework as one!
//     (*thread_id) = i;
//     pthread_create(thread_arr[i],NULL,(void*)thread_start,thread_id);
// }

// threadinfo_t *tinfo_arr = (threadinfo_t*)malloc(sizeof(*tinfo_arr) * thread_num);
// for (int i = 0; i < thread_num; i++)
// {
//     initializeThreadInfo(&tinfo_arr[i]);
//     // change thread id, is the following format correct?
//     changeThreadAndTID(&tinfo_arr[i],&thread_arr[i],i);
// }

Queue Implementation:

Queue createQueue(int maxSize)
{
    // no need for locks because we create the queues during the time we have 1 main thread!
    if (maxSize <= 0)
    {
        return NULL;
    }
    
    Queue q = (Queue)malloc(sizeof(*q));
    q->max_size = maxSize;
    q->current_size = 0;
    requests_t req;
    q->head = createNode(req,-1); // for programming purposes
    q->tail = createNode(req,-1); // for programming purposes
    (q->head)->next = q->tail;
    (q->tail)->prev = q->head;
    return q;
}

int isQueueFull(Queue queue)
{
    // may need locks to protect against clearing a spot from the queue right after the check is done
    if (queue->current_size == queue->max_size)
    {
        return TRUE; //full
    }
    return FALSE; // not full
}

int isQueueEmpty(Queue queue)
{
    // may need locks to protect against adding a spot to the queue right after the check is done
    if (queue->current_size == 0)
    {
        return TRUE; //empty
    }
    return FALSE; // not empty
}

int currentSizeOfQueue(Queue queue)
{
    // can't think of a case where this requires a lock because we can also just directly access the size if we want to!
    return(queue->current_size);
}

void enqueue(Queue queue, requests_t data, int id)
{
    // most definitely needs a lock to protect against queue becoming full right after one addition and then adding to it right after that!   
    if (isQueueFull(queue) == TRUE)
    {
        return;
    }
    
    Node to_add = createNode(data,id);
    Node current = queue->head;
    while (current->next != queue->tail)
    {
        current = current->next;
    }
    to_add->next = queue->tail; // last node in queue!
    to_add->prev = current;
    current->next = to_add;
    queue->current_size++;

}

void clearQueue(Queue queue)
{
    //I'm not sure if this requires a lock or not becaue i think that we will call this function only at the end when we're done and have one main thread so it doesn't matter if we place a lock or not
    if (isQueueEmpty(queue) == TRUE)
    {
        return;
    }
    Node current = queue->head->next;
    Node to_remove;
    while (current != queue->tail)
    {
        to_remove = current;
        current = current->next;
        free(to_remove);
    }
    queue->head->next = queue->tail;
    queue->tail->prev = queue->head;
    queue->current_size = 0;
}

void dequeueNode(Queue queue, int node_id)
{
    //most definitely need a lock to protect againt multiple deletions / queue becoming empty after one delete...
    Node current = queue->head->next;
    Node to_find = NULL;
    while (current != queue->tail)
    {
        if (current->node_id == node_id)
        {
            to_find = current;
            break;
        }
    }

    if (to_find != NULL)
    {
        (to_find->prev)->next = to_find->next;
        (to_find->next)->prev = to_find->prev;
        free(to_find);
        queue->current_size--;
    }
    
    
}

Node findNode(Queue queue, int node_id)
{
    //most definitely requires a lock to protect against someone else removing the node we're looking for while we're looking which may result in bugs if we found it and then the other thread deleted it!
    if (isQueueEmpty(queue) == TRUE)
    {
        return NULL; // this means that data doesn't exit in queue
    }
    
    Node current = queue->head;
    while (current->next != queue->tail)
    {
        if (current->node_id == node_id)
        {
            return current;
        }
    }
    
    return NULL; // this means that data doesn't exit in queue
}

void destroyQueue(Queue queue)
{
    clearQueue(queue);
    free(queue->head);
    free(queue->tail);
    free(queue);
}

*/
