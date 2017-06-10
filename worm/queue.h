#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

#include "worm.h"

typedef struct queue_t queue_t;
struct queue_t{
  	book_t* head;
		book_t* tail;
  	size_t size;
};

void enqueue(queue_t* queue, book_t* book);
book_t* dequeue(queue_t* queue);
queue_t* init_queue();
//void freeQueue(queue_t* queue);
bool isEmpty(queue_t* queue);

#endif
