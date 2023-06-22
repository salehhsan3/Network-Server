#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "request.h"
#include "segel.h"

pthread_mutex_t m_lock;
pthread_cond_t block_cond;
pthread_cond_t block_cond_main;

#define TRUE 1
#define FALSE 0

struct node_t
{
    requests_t *data;
    struct node_t *next;
    struct node_t *prev;
    int node_id; /* support for this feature has been removed. However, it could be useful.*/
};
typedef struct node_t *Node;

struct queue_t
{
    int capacity; /* the actual size(capacity) of the queue at any given moment*/
    int max_size; /* the maximum size that the queue can reach during its life-time*/
    int current_size;
    Node head;
    Node tail;
};
typedef struct queue_t *Queue;

requests_t *getData(Node node);
Node allocateNode(requests_t *req);

Queue createQueue(int capacity, int maxSize);
int isQueueEmpty(Queue queue);
int isQueueFull(Queue queue);
int capacityOfQueue(Queue queue);
int isQueueExpandable(Queue queue);
int ExpandQueue(Queue queue);
int currentSizeOfQueue(Queue queue);
void push(Queue queue, requests_t *data);
void pop(Queue queue);
Node findNode(Queue queue, int node_enum);
Node findNodeByData(Queue queue, requests_t *req_to_find);
int popRequest(Queue queue, requests_t *req_to_find);
int popRandom(Queue queue);
void destroyQueue(Queue queue);

#endif