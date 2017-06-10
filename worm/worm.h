#ifndef WORM_H
#define WORM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct book_t book_t;
struct book_t {
	size_t id;
	size_t author_id;
	size_t publisher_id;
	size_t* b_author_edges;
	size_t* b_citation_edges;
	size_t* b_publisher_edges;
	size_t n_author_edges;
	size_t n_citation_edges;
	size_t n_publisher_edges;
	size_t index;
	book_t* next;
	book_t* prev;
	//book_t* backptr;
};


typedef struct result_t {
	book_t** elements;
	size_t n_elements;
} result_t;

typedef struct node_t {
	size_t thread_id;
	book_t* nodes;
	size_t count;
	size_t id;
} node_t;

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


result_t* find_book(book_t* nodes, size_t count, size_t book_id);
result_t* find_books_by_author(book_t* nodes, size_t count, size_t author_id);
result_t* find_books_reprinted(book_t* nodes, size_t count, size_t publisher_id);
result_t* find_books_k_distance(book_t* nodes, size_t count, size_t book_id, uint16_t k);
result_t* find_shortest_distance(book_t* nodes, size_t count, size_t b1_id, size_t b2_id);
result_t* find_shortest_edge_type(book_t* nodes, size_t count, size_t a1_id, size_t a2_id);

#endif
