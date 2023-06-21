#include "queue.h"

requests_t *getData(Node node)
{
    return (node->data);
}

Node createNode(requests_t *req, int node_id)
{
    Node created_node = (Node)malloc(sizeof(*created_node));
    created_node->data = req;
    created_node->node_id = node_id;
    created_node->next = NULL;
    created_node->prev = NULL;
    return created_node;
}

Node findNode(Queue queue, int node_enum)
{
    // the enumeration starts from the head of the queue which is 1 and keeps going.
    Node current = queue->head->prev;
    int curr_enum = 1;

    while (current != queue->tail)
    {
        if ( curr_enum == node_enum)
            return current;

        current = current->prev;
        curr_enum++;
    }
    
    return NULL; 
}

Node findNodeByData(Queue queue, requests_t *req_to_find)
{
    Node current = queue->head->prev;

    while (current != queue->tail)
    {
        if(current->data->fd == req_to_find->fd)
        {
            return current;
        }
        current = current->prev;
    }
    
    return NULL; 
}

Queue createQueue(int capacity, int maxSize)
{
    if (capacity <= 0 || capacity < maxSize)
        return NULL;
    
    Queue q = (Queue)malloc(sizeof(*q));
    q->capacity = capacity;
    q->max_size = maxSize;
    q->current_size = 0;
    requests_t *req = NULL;
    q->head = createNode(req,-1); 
    q->tail = createNode(req,-1); 

    q->head->prev = q->tail;
    q->tail->next = q->head;

    q->head->next = NULL;
    q->tail->prev = NULL;

    return q;
}

int isQueueEmpty(Queue queue)
{
    return (queue->current_size == 0);
}

int isQueueFull(Queue queue)
{
    return (queue->current_size == queue->capacity);
}

int currentSizeOfQueue(Queue queue)
{
    return (queue->current_size);
}

int capacityofQueue(Queue queue)
{
    return queue->capacity;
}
int isQueueExpandable(Queue queue)
{
    return(queue->capacity < queue->max_size);
}

void ExpandQueue(Queue queue)
{
    if (isQueueExpandable(queue))
    {
        queue->capacity++;
    }
}

/* Insertion example: */

    //head <--> node_3  <--> node_2  <--> node_1  <--> tail
    
    //   Becomes to: 

    //head <--> node_3  <--> node_2  <--> node_1  <--> NEW <--> tail

    //next is <--
    //prev is -->
void enqueue(Queue queue, requests_t *data, int id)
{
    if (isQueueFull(queue) == TRUE)
    {
        return;
    }
    
    Node to_add = createNode(data, id);
    updateToUnAvailable(data);

    to_add->next = queue->tail->next;   //node_1 <-- NEW
    to_add->prev = queue->tail;         //NEW --> tail

    queue->tail->next->prev = to_add;   //node_1 --> NEW
    queue->tail->next = to_add;         //NEW <-- TAIL

    queue->current_size++;
}

void dequeue(Queue queue)
{
    while ( isQueueEmpty(queue) )
    {
        pthread_cond_wait( &block_cond, &m_lock );
    }
    
    Node to_remove = queue->head->prev; 
    requests_t* data = to_remove->data;
    to_remove->prev->next = to_remove->next;
    to_remove->next->prev = to_remove->prev;
    free(to_remove);
    queue->current_size--;
    updateToAvailable(data);
}

int dequeueRequest(Queue queue, requests_t *req_to_find)
{
    
    Node to_find = findNodeByData(queue, req_to_find);
    if(to_find == NULL)
        return -1;

    requests_t* data = to_find->data;
    to_find->prev->next = to_find->next;
    to_find->next->prev = to_find->prev;
    queue->current_size--;
    free(to_find);
    updateToAvailable(data);
    return 0;
}

void clearQueue(Queue queue)
{
    if (isQueueEmpty(queue) == TRUE)
    {
        return;
    }
    Node current = queue->head->prev;
    Node to_remove;
    requests_t* data;
    while (current != queue->tail)
    {
        to_remove = current;
        current = current->prev;
        data = current->data;
        updateToAvailable(data);
        free(to_remove);
    }
    queue->head->prev = queue->tail;
    queue->tail->next = queue->head;
    queue->current_size = 0;
}

void destroyQueue(Queue queue)
{
    clearQueue(queue);
    free(queue->head);
    free(queue->tail);
    free(queue);
}
