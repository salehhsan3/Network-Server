#include "queue.h"

Node createNode(requests_t *req, int node_id)
{
    Node created_node = (Node)malloc(sizeof(*created_node));
    created_node->data = req;
    created_node->node_id = node_id;
    created_node->next = NULL;
    created_node->prev = NULL;
    return created_node;
}

requests_t *getData(Node node)
{
    return (node->data);
}

int getId(Node node)
{
    return node->node_id;
}

Queue createQueue(int size, int maxSize)
{
    if (size <= 0 || size < maxSize)
        return NULL;
    
    Queue q = (Queue)malloc(sizeof(*q));
    q->size = size;
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

int isQueueFull(Queue queue)
{
    return (queue->current_size == queue->size);
}

int isQueueEmpty(Queue queue)
{
    return (queue->current_size == 0);
}

int currentSizeOfQueue(Queue queue)
{
    return (queue->current_size);
}

int capacityofQueue(Queue queue)
{
    return queue->size;
}
int isQueueExpandable(Queue queue)
{
    return(queue->size < queue->max_size);
}

void ExpandQueue(Queue queue)
{
    if (isQueueExpandable(queue))
    {
        queue->size++;
    }
}

void enqueue(Queue queue, requests_t *data, int id)
{
    if (isQueueFull(queue) == TRUE)
    {
        return;
    }
    
    Node to_add = createNode(data, id);

    /* Insertion example: */

    //head <--> node_3  <--> node_2  <--> node_1  <--> tail
    
    //   Becomes to: 

    //head <--> node_3  <--> node_2  <--> node_1  <--> NEW <--> tail

    //next is <--
    //prev is -->


    to_add->next = queue->tail->next;   //node_1 <-- NEW
    to_add->prev = queue->tail;         //NEW --> tail

    queue->tail->next->prev = to_add;   //node_1 --> NEW
    queue->tail->next = to_add;         //NEW <-- TAIL

    queue->current_size++;

    //pthread_cond_signal( &(queue->cond_var) ); // signal that we're done adding new item (if queue was empty its not now!)
    //pthread_mutex_unlock( &(queue->m_lock) );

}

void dequeue(Queue queue)
{
    while ( isQueueEmpty(queue) )
    {
        pthread_cond_wait( &block_cond, &m_lock ); // as long as the queue is empty keep waiting!
    }
    
    Node to_remove = queue->head->prev; // by now queue must have at least one item! and it is also the oldest
    to_remove->prev->next = to_remove->next;
    to_remove->next->prev = to_remove->prev;
    free(to_remove);
    queue->current_size--;
    resetIndices(queue);
}

Node findNode(Queue queue, int node_enum)
{
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

int isElementInQueue(Queue queue, int node_id)
{
    return (findNode(queue, node_id) != NULL);
}

int isElementInQueueByData(Queue queue, requests_t *req_to_find)
{
    return (findNodeByData(queue, req_to_find) != NULL);
}

void dequeueElement(Queue queue, int id)
{
    resetIndices(queue);
    
    Node to_find = findNode(queue, id);
    while ( to_find == NULL )
    {
        pthread_cond_wait( &block_cond, &m_lock );
    }

    to_find = findNode(queue, id);

    to_find->prev->next = to_find->next;
    to_find->next->prev = to_find->prev;
    queue->current_size--;
    free(to_find);
    
}

int dequeueElementByRequest(Queue queue, requests_t *req_to_find)
{
    
    Node to_find = findNodeByData(queue, req_to_find);
    if(to_find == NULL)
        return -1;

    to_find->prev->next = to_find->next;
    to_find->next->prev = to_find->prev;
    queue->current_size--;
    free(to_find);

    resetIndices(queue);
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
    while (current != queue->tail)
    {
        to_remove = current;
        current = current->prev;
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

void resetIndices(Queue queue)
{
    Node current = queue->head->prev;
    int curr_id = 1;
    while (current != queue->tail)
    {
        current->node_id = curr_id;
        curr_id++;
        current = current->prev;   
    }
}

void printQueueData(Queue q, char* qName)
{
    printf("Printing %s queue data(FD):\n", qName);
    printf("HEAD --> ");
    
    Node current = q->head->prev;
    while (current != q->tail)
    {
        printf("%d --> ", current->data->fd);
        current = current->prev;
    }
    printf("TAIL\n");
}