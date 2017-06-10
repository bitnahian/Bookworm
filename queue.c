#include "worm.h"
#include "queue.h"

void enqueue(queue_t* queue, book_t* book) // O(1)
{
	// Increase the queue at the head
	if(queue->head == NULL) // No lockers have been queued
	{
		queue->head = book;
		queue->tail = book;
		queue->size++;
		return;
	}
	queue->head->prev = book;
	book->next = queue->head;
	queue->head = book;
	queue->size++;
}

book_t* dequeue(queue_t* queue) // O(1)
{
	// Decrease the queue at the tail
	if (queue->tail == NULL) return NULL; // No lockers have been queued
	book_t* temp = queue->tail;
	if(queue->size > 1)
	{
		queue->tail = queue->tail->prev;
		queue->tail->next = NULL;
	}
	else
	{
		queue->head = NULL;
		queue->tail = NULL;
	}
	queue->size--;
	return temp;
}

queue_t* init_queue() {
	queue_t* queue = malloc(sizeof(queue_t)); // remember to free later
	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;

	return queue;
}

bool isEmpty(queue_t* queue)
{
	if (queue->head == NULL && queue->tail == NULL) return true;
	else return false;
}
