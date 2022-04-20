#include "queue.h"
#include "private.h"

static char is_empty(queue_t* queue) { return queue->count == 0; }

static char is_full(queue_t* queue) { return queue->count == queue->capacity; }

queue_t* queue_new(size_t size, size_t allocation_size)
{
    queue_t* queue = (queue_t*)calloc(1, sizeof(queue_t));
    CHECK(queue, NULL, "Couldn't allocate memory for queue");

    queue->data = (void*)malloc(allocation_size);
    queue->rear = -1;
    queue->capacity = size;

    return queue;
}

void queue_enqueue(queue_t* queue, void* data)
{
    if (is_full(queue))
    {
        printf("queue is full!\n");
        return;
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->data[queue->rear] = data;
    queue->count++;
}

void queue_dequeue(queue_t* queue)
{
    if (is_empty(queue))
    {
        printf("queue is empty!\n");
        return;
    }

    queue->front = (queue->front + 1) % queue->capacity;
    queue->count--;
}

void* queue_peek(queue_t* queue) { return queue->data[queue->front]; }