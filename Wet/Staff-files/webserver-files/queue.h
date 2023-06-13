#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "request.h"

pthread_mutex_t m_lock;
pthread_cond_t block_cond;
pthread_cond_t block_cond_main;
#define TRUE 1
#define FALSE 0

struct node_t {
    int node_id;
    requests_t *data; 
    struct node_t* next;
    struct node_t* prev;
};
typedef struct node_t* Node;

struct queue_t{
    int size;       /* the actual size(capacity) of the queue at any given moment*/
    int max_size;   /* the maximum size that the queue can reach during its life-time*/
    int current_size;
    Node head;
    Node tail;
};
typedef struct queue_t* Queue;


Node createNode(requests_t *req, int node_id);
requests_t *getData(Node node);
int getId(Node node);

Queue createQueue(int size, int maxSize);
int isQueueFull(Queue queue); // 1 is true, 0 is false
int isQueueEmpty(Queue queue);
int currentSizeOfQueue(Queue queue);
int capacityofQueue(Queue queue);
int isQueueExpandable(Queue queue);
void ExpandQueue(Queue queue);
void enqueue(Queue queue, requests_t *data, int id);
void dequeue(Queue queue);
Node findNode(Queue queue, int node_enum);
Node findNodeByData(Queue queue, requests_t *req_to_find);
int isElementInQueue(Queue queue, int node_id);
int isElementInQueueByData(Queue queue, requests_t *req_to_find);
void dequeueElement(Queue queue, int id);
int dequeueElementByRequest(Queue queue, requests_t *req_to_find);
void clearQueue(Queue queue);
void destroyQueue(Queue queue);
void resetIndices(Queue queue);
pthread_mutex_t *getQueueLock(Queue q);
pthread_cond_t *getQueueConditionVariable(Queue q);
void printQueueData(Queue q, char* qName);

#endif