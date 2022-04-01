#ifndef QUEUE_H
#define QUEUE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct queue{
    int count;
    int front;
    int rear;
    size_t capacity;
    void** data;
}queue_t;

queue_t* queue_new(size_t size, size_t allocation_size);
void queue_enqueue(queue_t* queue, void* data);
void queue_dequeue(queue_t* queue);
void* queue_peek(queue_t* queue);
void queue_destroy(queue_t* queue);

#endif