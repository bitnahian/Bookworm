#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "worm.h"

#define INFINITY 21
#define MIN(X, Y) (((X->n_elements) < (Y->n_elements)) ? (X) : (Y))

#define remainder (count%g_nthreads)
#define chunks (count/g_nthreads)

size_t g_nthreads = 4;
bool found = false;
//pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;


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

void freeQueue(queue_t* queue)
{
	//book_t* temp = NULL;
//	while((temp = dequeue(queue)) != NULL)
	//{
	//	free(temp);
//	}
	free(queue);
}

bool isEmpty(queue_t* queue)
{
	if (queue->head == NULL && queue->tail == NULL) return true;
	else return false;
}

// Initialises a result_t* struct
result_t* init_result()
{
	result_t* result = (result_t*)malloc(sizeof(result_t));
	result->elements = (book_t**)malloc(sizeof(result_t));
	result->n_elements = 0;
	return result;
}

// Returns the first address of the book_t* node it finds. Signals other threads to stop.
void *find_book_worker_function(void *arg)
{
	// Make life easy *wink wink*
	node_t* data = (node_t *)arg;
	size_t my_id = data->thread_id;
	book_t* nodes = data->nodes;
	size_t count = data->count;
	size_t book_id = data->id;
	book_t* node_to_find = NULL;

	int start = my_id*chunks;
	int end = start + chunks;

	// Account for non divisible counts
	if(end == (count-remainder)) end += remainder;

	// Break through the loop if the id is found and set found to true
	for(int i = start; i < end; ++i)
	{
		if(found) break;
		if((nodes + i)->id == book_id)
		{
			node_to_find = nodes + i;
			//pthread_mutex_lock(&locker);
			found = true;
			//pthread_mutex_unlock(&locker);
			break;
		}
	}
	return (void *)node_to_find;
}

// Returns result set containing book with given id.
result_t* find_book(book_t* nodes, size_t count, size_t book_id)
{
	// Initialize a result struct
	result_t* result = init_result();
	pthread_t threads[g_nthreads];
	node_t* arg[g_nthreads];

	int created = 0;

	// Create the threads and pass in the arguments
	for (int i = 0; i < g_nthreads; ++i)
	{
		arg[i] = (node_t*)malloc(sizeof(node_t));
		node_t* argp = arg[i];
		argp->thread_id = i;
		argp->nodes = nodes;
		argp->count = count;
		argp->id = book_id;

		created += !pthread_create(threads + i, NULL, find_book_worker_function, argp);
	}

	// Error checking if all threads were unable to be created
	if (created < g_nthreads) {
		fprintf(stderr, "Warning: could not create %zu threads (%d created)\n",
			g_nthreads, created);
	}

	// Join threads and process the result obtained from the threads
	for (int i = 0; i < created; ++i) {
		book_t *data;
		pthread_join(threads[i], (void**)&data);
		if(data != NULL)
		{
			result->elements[0] = data;
			result->n_elements++;
		}
		free(arg[i]);
	}
	found = false; // Set found to false for reuse
	return result;
}

// Returns result set containing books with the given author
void *find_author_worker_function(void *arg)
{
	// Make life easy *wink wink*
	node_t* data = (node_t *)arg;
	size_t my_id = data->thread_id;
	book_t* nodes = data->nodes;
	size_t count = data->count;
	size_t author_id = data->id;
	result_t* result = init_result();

	int start = my_id*chunks;
	int end = start + chunks;

	// Account for non divisible counts
	if(end == (count-remainder)) end += remainder;

	size_t lcount = 0;
	for(int i = start; i < end; ++i)
	{
		if((nodes + i)->author_id == author_id)
		{
			if(lcount > 0)
				result->elements = realloc(result->elements, sizeof(book_t**)*(lcount+1));
			result->elements[lcount] = (nodes+i);
			result->n_elements++;
			lcount++;
		}
	}
	free(arg);
	return (void *)result;
}

// Returns result set containing books by given author.
result_t* find_books_by_author(book_t* nodes, size_t count, size_t author_id) {

	pthread_t threads[g_nthreads];
	node_t* arg[g_nthreads];
	result_t* result = init_result();
	int created = 0;

	// Create the threads and pass in the arguments
	for (int i = 0; i < g_nthreads; ++i)
	{
		arg[i] = (node_t*)malloc(sizeof(node_t));
		node_t* argp = arg[i];
		argp->thread_id = i;
		argp->nodes = nodes;
		argp->count = count;
		argp->id = author_id;

		created += !pthread_create(threads + i, NULL, find_author_worker_function, argp);
	}


	// Error checking if all threads were unable to be created
	if (created < g_nthreads) {
		fprintf(stderr, "Warning: could not create %zu threads (%d created)\n",
			g_nthreads, created);
	}

	// Join threads and process the result obtained from the threads
	size_t lcount = 0;
	for (int i = 0; i < created; ++i)
	{
		result_t* data;
		pthread_join(threads[i], (void**)&data);
		if(data!= NULL && data->n_elements > 0)
		{
			size_t n_elements = data->n_elements;
			result->elements = realloc(result->elements, sizeof(book_t**)*(lcount+n_elements));
			memcpy(result->elements + lcount, data->elements, sizeof(book_t**)*n_elements);
			lcount += n_elements;
		}
		free(data->elements);
		free(data);
	}
	result->n_elements = lcount;
	return result;
}


void *find_publisher_worker_function(void *arg)
{
	// Make life easy *wink wink*
	node_t* data = (node_t *)arg;
	size_t my_id = data->thread_id;
	book_t* nodes = data->nodes;
	size_t count = data->count;
	size_t pub_id = data->id;
	result_t* result = init_result();

	int start = my_id*chunks;
	int end = start + chunks;

	// Account for non divisible counts
	if(end == (count-remainder)) end += remainder;

	size_t lcount = 0;
	for(int i = start; i < end; ++i)
	{
		if((nodes + i)->publisher_id == pub_id)
		{
			size_t curr_book_id = (nodes+i)->id;
			for(int j = 0; j < count; ++j)
			{
				if((nodes+j)->id == curr_book_id && (nodes+j)->publisher_id != pub_id)
				{
					//printf("reached\n");
					if(lcount > 0)
						result->elements = realloc(result->elements, sizeof(book_t**)*(lcount+1));
					result->elements[lcount] = (nodes+j);
					result->n_elements++;
					lcount++;
				}
			}

		}
	}

	free(arg);
	return (void *)result;
}


// Returns result set containing books that have been reprinted by a different publisher.
result_t* find_books_reprinted(book_t* nodes, size_t count, size_t publisher_id) {

	pthread_t threads[g_nthreads];
	node_t* arg[g_nthreads];
	result_t* result = init_result();
	int created = 0;

	// Create the threads and pass in the arguments
	for (int i = 0; i < g_nthreads; ++i)
	{

		arg[i] = (node_t*)malloc(sizeof(node_t));
		node_t* argp = arg[i];
		argp->thread_id = i;
		argp->nodes = nodes;
		argp->count = count;
		argp->id = publisher_id;

		created += !pthread_create(threads + i, NULL, find_publisher_worker_function, argp);
	}


	// Error checking if all threads were unable to be created
	if (created < g_nthreads) {
		fprintf(stderr, "Warning: could not create %zu threads (%d created)\n",
			g_nthreads, created);
	}

	// Join threads and process the result obtained from the threads
	size_t lcount = 0;
	for (int i = 0; i < created; ++i)
	{
		result_t* data;
		pthread_join(threads[i], (void**)&data);
		if(data!= NULL && data->n_elements > 0)
		{
			size_t n_elements = data->n_elements;
			result->elements = realloc(result->elements, sizeof(book_t**)*(lcount+n_elements));
			memcpy(result->elements + lcount, data->elements, sizeof(book_t**)*n_elements);
			lcount += n_elements;
		}
		free(data->elements);
		free(data);
	}
	result->n_elements = lcount;

	// Result now contains all the set of books with the same publisher id.
	// With this set, I have to look at all the books in the author edges of the respective books I found
	// This makes searching for another book with the same id easier


	return result;
}


// Returns result set containing books that are k distance from given book.
result_t* find_books_k_distance(book_t* nodes, size_t count, size_t book_id, uint16_t k) {

	if(k > count)
		k = count;
	result_t* result = init_result();
	result->elements = (book_t**)malloc(sizeof(book_t*));
	// Find book. Think about doing this with threads later
	size_t book_index = -1;
	for(int i = 0; i < count; ++i)
	{
		(nodes+i)->index = i;
		if((nodes+i)->id == book_id)
		{
			book_index = (nodes+i)->index;
			break;
		}
	}
	// book_index not found
	if(book_index == -1) return result;

	bool flag[count]; // flag[] to keep track of which nodes were visited
	int prev[count]; // prev[] to keep track of which nodes came before the kth indexed node where prev[k]

	// Perform BFS
	while(k >= 0)
	{

	}
	return result;
}

result_t* find_shortest_distance(book_t* nodes, size_t count, size_t b1_id, size_t b2_id)
{
	result_t* result = init_result();
	int i = 0;
	size_t b1_index = -1;
	size_t b2_index = -1;
	bool flag[count]; // flag[] to keep track of which nodes were visited
	int prev[count]; // prev[] to keep track of which nodes came before the kth indexed node where prev[k]
	bool b2_found = false;
	// Find b1_id
	for(i = 0; i < count; ++i)
	{
		flag[i] = false;
		prev[i] = -1; // Set all previous values to -1
		book_t* curr_node = (nodes+i);
		curr_node->index = i;
		if(curr_node->id == b1_id)
		{
			b1_index = i;
			//printf("%zu\n", b1_index);
		}
		if(curr_node->id == b2_id) {
			b2_index = i;
			//printf("%zu\n", b2_index);
		}
	}
	if(b1_index == -1) return result;
	if(b2_index == -1) return result;
	if(b1_id == b2_id) return result;
	// BFS FUNCTION
	queue_t* queue = init_queue(); // Initialise the queue
	book_t* v = (nodes+b1_index); // Set v to first node
	flag[b1_index] = true;
	enqueue(queue, v);
	while(!isEmpty(queue))
	{
		v = dequeue(queue);
		size_t current_index = v->index;
		// Visit all the neighbours of u with all the edges and push them into the queue
		size_t n_edges = (nodes+current_index)->n_author_edges + (nodes+current_index)->n_publisher_edges + (nodes+current_index)->n_citation_edges;
		for(int j = 0; j < n_edges; ++j)
		{
			int index = 0;
			if(j >= 0 && j < (nodes+current_index)->n_author_edges)
				index = *((nodes+current_index)->b_author_edges+j);
			else if( j >= (nodes+current_index)->n_author_edges && j < (nodes+current_index)->n_author_edges + (nodes+current_index)->n_citation_edges)
				index = *((nodes+current_index)->b_citation_edges+(j - (nodes+current_index)->n_author_edges));
			else
				index = *((nodes+current_index)->b_publisher_edges+j - ((nodes+current_index)->n_author_edges + (nodes+current_index)->n_citation_edges));

			if(flag[index] == false)
			{
				flag[index] = true;
				enqueue(queue, nodes + index);
				prev[index] = current_index;
				if(index == b2_index)
				{
					b2_found = true;
					break;
				}
			}
		}
		if(b2_found) break;
		/*
		for(int j = 0; j < (nodes+current_index)->n_citation_edges; ++j)
		{
			int index = *((nodes+current_index)->b_citation_edges+j);
			if(flag[index] == false)
			{
				flag[index] = true;
				enqueue(queue, nodes + index);
				//(nodes+index)->back_pointer = (nodes+current_index);
				prev[index] = current_index;
				if(index == b2_index)
				{
					b2_found = true;
					break;
				}
			}
		}
		if(b2_found) break;
		for(int j = 0; j < (nodes+current_index)->n_publisher_edges; ++j)
		{
			int index = *((nodes+current_index)->b_publisher_edges+j);
			if(flag[index] == false)
			{
				flag[index] = true;
				enqueue(queue, nodes + index);
				prev[index] = current_index;
				if(index == b2_index)
				{
					b2_found = true;
					break;
				}
			}
		}
		if(b2_found) break;
		*/
	}
	free(queue);
	// Return empty set if there was no path
	if(prev[b2_index] == -1)
		return result;
	// Backtrack to find the shortest path
	book_t** elements = (book_t**)malloc(sizeof(book_t*));
	elements[0] = (nodes+b2_index);
	int mcount = 1;
	// *** Backtracking Algorithm ***
	int size = 1;
	while(prev[b2_index] != -1)
	{
		mcount++;
		b2_index = prev[b2_index];
		if(size < mcount) size = size*2;
		elements = realloc(elements, sizeof(book_t*)*size);
		elements[mcount-1] = (nodes+b2_index);
	}
	result->elements = realloc(result->elements, sizeof(book_t*)*mcount);
	for(int m = 0; m < mcount; ++m)
	{
		result->elements[m] = elements[mcount-m-1];
	}
	result->n_elements = mcount;
	free(elements);

	return result;
}

// Returns result set containing books in shortest path of two edges types between author 1 and 2.
result_t* find_shortest_edge_type(book_t* nodes, size_t count, size_t a1_id, size_t a2_id) {

	// TODO
	return NULL;
}
/*
result_t* author_BFS(book_t* nodes, size_t count, size_t b1_id, size_t b2_id)
{
	// Initialize a result struct
	result_t* result = init_result();
	int i = 0;
	size_t b1_index = 0;
	size_t b2_index = 0;
	bool flag[count]; // flag[] to keep track of which nodes were visited
	int prev[count]; // prev[] to keep track of which nodes came before the kth indexed node where prev[k]

	// Find b1_id
	for(i = 0; i < count; ++i)
	{
		flag[i] = false;
		prev[i] = -1; // Set all previous values to -1
		(nodes+i)->index = i;
		if((nodes+i)->id == b1_id) {
			b1_index = i;
		}
		if((nodes+i)->id == b2_id) {
			b2_index = i;
		}
	}
	// BFS FUNCTION
	queue_t* queue = init_queue(); // Initialise the queue
	book_t* v = (nodes+b1_index); // Set v to first node
	flag[b1_index] = true;
	enqueue(queue, v);
	while(!isEmpty(queue))
	{
		book_t* u = dequeue(queue);
		size_t current_index = u->index;
		// Visit all the neighbours of u
		for(int j = 0; j < (nodes+current_index)->n_author_edges; ++j)
		{
			int index = *((nodes+current_index)->b_author_edges+j);
			if(flag[index] == false)
			{
			flag[index] = true;
			enqueue(queue, nodes + index);
			prev[index] = current_index;
			if(index == b2_index) break;
			}
		}
	}
	free(queue);
	// Backtrack to find the shortest path
	book_t** elements = (book_t**)malloc(sizeof(book_t*));
	elements[0] = (nodes+b2_index);
	//printf("reached2\n" );
	int mcount = 1;
	// *** Backtracking Algorithm ***
	while(prev[b2_index] != -1)
	{
		mcount++;
		b2_index = prev[b2_index];
		elements = realloc(elements, sizeof(book_t*)*mcount);
		elements[mcount-1] = (nodes+b2_index);
	}
	result->elements = realloc(result->elements, sizeof(book_t*)*mcount);
	for(int m = 0; m < mcount; ++m)
	{
		result->elements[m] = elements[mcount-m-1];
	}

	result->n_elements = mcount;
	free(elements);

	return result;
}
result_t* publisher_BFS(book_t* nodes, size_t count, size_t b1_id, size_t b2_id)
{
	// Initialize a result struct
	result_t* result = init_result();
	int i = 0;
	size_t b1_index = 0;
	size_t b2_index = 0;
	bool flag[count];
	int prev[count];

	// Find b1_id
	for(i = 0; i < count; ++i)
	{
		flag[i] = false;
		prev[i] = -1; // Set all previous values to -1
		(nodes+i)->index = i;
		if((nodes+i)->id == b1_id) {
			b1_index = i;
		}
		if((nodes+i)->id == b2_id) {
			b2_index = i;
		}
	}
	// BFS FUNCTION
	queue_t* queue = init_queue(); // Initialise the queue
	book_t* v = (nodes+b1_index); // Set v to first node
	flag[b1_index] = true;
	enqueue(queue, v);
	while(!isEmpty(queue))
	{
		book_t* u = dequeue(queue);
		size_t current_index = u->index;
		for(int j = 0; j < (nodes+current_index)->n_publisher_edges; ++j)
		{
			int index = *((nodes+current_index)->b_publisher_edges+j);
			if(flag[index] == false)
			{
			flag[index] = true;
			enqueue(queue, nodes + index);
			prev[index] = current_index;
			if(index == b2_index) break;
			}
		}
	}
	free(queue);
	book_t** elements = (book_t**)malloc(sizeof(book_t*));
	elements[0] = (nodes+b2_index);
	int mcount = 1;
	while(prev[b2_index] != -1)
	{
		mcount++;
		b2_index = prev[b2_index];
		elements = realloc(elements, sizeof(book_t*)*mcount);
		elements[mcount-1] = (nodes+b2_index);
	}
	result->elements = realloc(result->elements, sizeof(book_t*)*mcount);
	for(int m = 0; m < mcount; ++m)
	{
		result->elements[m] = elements[mcount-m-1];
	}

	result->n_elements = mcount;
	free(elements);

	return result;
}
result_t* citations_BFS(book_t* nodes, size_t count, size_t b1_id, size_t b2_id)
{
	// Initialize a result struct
	result_t* result = init_result();
	int i = 0;
	size_t b1_index = 0;
	size_t b2_index = 0;
	bool flag[count];
	int prev[count];

	// Find b1_id
	for(i = 0; i < count; ++i)
	{
		flag[i] = false;
		prev[i] = -1; // Set all previous values to -1
		(nodes+i)->index = i;
		if((nodes+i)->id == b1_id) {
			b1_index = i;
		}
		if((nodes+i)->id == b2_id) {
			b2_index = i;
		}
	}
	// BFS FUNCTION
	queue_t* queue = init_queue(); // Initialise the queue
	book_t* v = (nodes+b1_index); // Set v to first node
	flag[b1_index] = true;
	enqueue(queue, v);
	while(!isEmpty(queue))
	{
		book_t* u = dequeue(queue);
		size_t current_index = u->index;
		for(int j = 0; j < (nodes+current_index)->n_citation_edges; ++j)
		{
			int index = *((nodes+current_index)->b_citation_edges+j);
			if(flag[index] == false)
			{
			flag[index] = true;
			enqueue(queue, nodes + index);
			prev[index] = current_index;
			if(index == b2_index) break;
			}
		}
	}
	free(queue);
	book_t** elements = (book_t**)malloc(sizeof(book_t*));
	elements[0] = (nodes+b2_index);
	int mcount = 1;
	while(prev[b2_index] != -1)
	{
		mcount++;
		b2_index = prev[b2_index];
		elements = realloc(elements, sizeof(book_t*)*mcount);
		elements[mcount-1] = (nodes+b2_index);
	}
	result->elements = realloc(result->elements, sizeof(book_t*)*mcount);
	for(int m = 0; m < mcount; ++m)
	{
		result->elements[m] = elements[mcount-m-1];
	}
	if(mcount == 1) mcount = INFINITY;
	result->n_elements = mcount;
	free(elements);

	return result;
}


// Returns result set containing books in shortest path between book 1 and 2.
result_t* find_shortest_distance(book_t* nodes, size_t count, size_t b1_id, size_t b2_id) {

	//pthread_t threads[g_nthreads];
	//node_t* arg[g_nthreads];
	result_t* r1 = author_BFS(nodes, count, b1_id, b2_id);
	result_t* r2 = publisher_BFS(nodes, count, b1_id, b2_id);
	result_t* r3 = citations_BFS(nodes, count, b1_id, b2_id);

	result_t* r4 = MIN(r1,r2);
	result_t* r5 = MIN(r3,r4);

	if(r5 != r1)
	{
		free(r1->elements);
		free(r1);
	}
	if(r5 != r2)
	{
		free(r2->elements);
		free(r2);
	}
	if(r5 != r3)
	{
		free(r3->elements);
		free(r3);
	}

	return r5;
}
*/
