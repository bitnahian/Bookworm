#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "worm.h"

#define TEN_MILLION 10000000000

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

size_t dequeue(queue_t* queue) // O(1)
{
	// Decrease the queue at the tail
	if (queue->tail == NULL) return -1	; // No lockers have been queued
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
	return temp->index;
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

// Initialises a result_t* struct
result_t* init_result()
{
	result_t* result = (result_t*)malloc(sizeof(result_t));
	result->n_elements = 0;
	return result;
}

// Returns result set containing book with given id.
result_t* find_book(book_t* nodes, size_t count, size_t book_id)
{
	//if(count < TEN_MILLION)
	//{
		//sequential
		result_t* result = init_result();
		result->elements = (book_t**)malloc(sizeof(book_t*));
		//size_t i = 0;
		book_t* book;
		//#pragma clang loop unroll_count(16)
		for(book = nodes; book < nodes + count; ++book)
		{
			if(book->id == book_id)
			{
				result->elements[0]=book;
				result->n_elements++;
				return result;
			}
		}
		return result;
	}
	/*}
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
*/
// Returns result set containing books by given author.
result_t* find_books_by_author(book_t* nodes, size_t count, size_t author_id)
{
	/*
	if(count < TEN_MILLION)
	{
		*/	//sequential
			bool auth_found = false;
			result_t* result = init_result();
			size_t size = 10;
			result->elements = (book_t**)malloc(sizeof(book_t*)*size);
			//#pragma unroll
			//size_t i = 0;
			book_t* tempbooks;
			for(tempbooks = nodes; tempbooks < nodes + count; ++tempbooks)
			{
				if(!auth_found && tempbooks->author_id == author_id)
				{
					result->elements[0]=tempbooks;
					auth_found = true;
					break;
				}
			}
			if(!auth_found) return result;

			// Look through the author edges
			size_t mcount = 1;
			size_t n_author_edges = tempbooks->n_author_edges;
			size_t* b_author_edges;
			size_t* curr_edge = tempbooks->b_author_edges;
			for(b_author_edges = curr_edge ; b_author_edges < curr_edge + n_author_edges; ++b_author_edges)
			{
				mcount++;
				if(mcount > size)
				{
					size = size*2;
					result->elements = realloc(result->elements, sizeof(book_t**)*size);
				}
				result->elements[mcount-1] = (nodes + *(b_author_edges));
			}
			result->n_elements = mcount;
			return result;
	}
	/*
	// Threaded
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
*/
// Returns result set containing books that have been reprinted by a different publisher.
result_t* find_books_reprinted(book_t* nodes, size_t count, size_t publisher_id) {
	/*if(count < TEN_MILLION)
	{
	*/	//sequential
	result_t* result = init_result();
	size_t size = 10;
	result->elements = (book_t**)malloc(sizeof(book_t*)*size);
	size_t lcount = 1;
	int i = 0;
	int j = 0;
	for(i = 0; i < count; ++i)
	{
		if((nodes + i)->publisher_id == publisher_id)
		{
			size_t curr_book_id = (nodes+i)->id;
			size_t* auth_edge = (nodes+i)->b_author_edges;
			for(j = 0; j < (nodes+i)->n_author_edges; ++j)
			{
				if((nodes + auth_edge[j])->id == curr_book_id
				&& (nodes+ auth_edge[j])->publisher_id != publisher_id)
				{
					if(lcount > size)
					{
						size = size*2;
						result->elements = realloc(result->elements, sizeof(book_t**)*size);
					}
					result->elements[lcount-1] = (nodes+ auth_edge[j]);
					result->n_elements++;
					lcount++;
				}
			}
		}
	}

	return result;
}
	/*
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
*/

// Returns result set containing books that are k distance from given book.
result_t* find_books_k_distance(book_t* nodes, size_t count, size_t book_id, uint16_t k) {
	result_t* result = init_result();
	size_t mcount = 1;
	size_t size = 100;
	bool book_found = false;
	result->elements = (book_t**)malloc(sizeof(book_t*)*size);
	// Find book and initialize stuff
	int16_t dis[count]; // pun pun pun pun pun pun pun
	size_t book_index = -1;
	size_t i = 0;
	for(i = 0; i < count; ++i)
	{
		dis[i] = -1;
		(nodes+i)->index = i;
		if(!book_found && (nodes+i)->id == book_id)
			{
				book_index = i;
				book_found = true;
			}
	}
	// book_index not found
	if(book_index == -1) return result;
	result->elements[0] = (nodes+book_index);
	// BFS FUNCTION
	bool reached = false;
	dis[book_index] = 0;
	size_t index = 0;
	size_t current_index = 0;
	size_t *b_citation_edges;
	queue_t* queue = init_queue(); // Initialise the queue
	book_t* v = nodes + book_index; // Set v to first node
	enqueue(queue, v);
	while(!isEmpty(queue) && !reached)
	{
		current_index = dequeue(queue);
		// Visit all the neighbours of u with all the edges and push them into the queue
		for(b_citation_edges = (nodes+current_index)->b_citation_edges;
		b_citation_edges < ((nodes+current_index)->b_citation_edges + (nodes+current_index)->n_citation_edges);++b_citation_edges)
		{
			if(dis[*(b_citation_edges)] == -1)
			{
				index = *(b_citation_edges);
				dis[index] = dis[current_index] + 1;
				if(dis[index] <= k)
				{
					mcount++;
					if(mcount > size)
					{
						size = size*2;
						result->elements = realloc(result->elements, sizeof(book_t*)*size);
					}
					result->elements[mcount-1] = (nodes+index);
					enqueue(queue, nodes + index);
				}
				else if(dis[index] > k){
					reached = true;
					break;
				}
			}
		}
	}
	free(queue);
	result->n_elements = mcount;
	return result;
}

result_t* find_shortest_distance(book_t* nodes, size_t count, size_t b1_id, size_t b2_id)
{
	result_t* result = init_result();
	result->elements = (book_t**)malloc(sizeof(book_t**));
	size_t i = 0;
	size_t b1_index = -1;
	size_t b2_index = -1;
	bool flag[count]; // flag[] to keep track of which nodes were visited
	int prev[count]; // prev[] to keep track of which nodes came before the kth indexed node where prev[k]
	bool b1f = false;
	bool b2f = false;
	bool b2_found = false;
	// Find b1_id
	//#pragma unroll
	for(i = 0; i < count; ++i)
	{
		flag[i] = false;
		prev[i] = -1; // Set all previous values to -1
		book_t* curr_node = (nodes+i);
		curr_node->index = i;
		if(!b1f && curr_node->id == b1_id)
		{
			b1_index = i;
			b1f = true;
		}
		if(!b2f && curr_node->id == b2_id)
		{
			b2_index = i;
			b2f = true;
		}
	}
	if(b1_index == -1) return result;
	if(b2_index == -1) return result;
	// BFS FUNCTION
	queue_t* queue = init_queue(); // Initialise the queue
	book_t* v = (nodes+b1_index); // Set v to first node
	flag[b1_index] = true;
	size_t* curr_edge;
	enqueue(queue, v);
	while(!isEmpty(queue))
	{

		size_t current_index = dequeue(queue);
		// Visit all the neighbours of u with all the edges and push them into the queue
		size_t n_author_edges = (nodes+current_index)->n_author_edges;
		for(curr_edge = (nodes+current_index)->b_author_edges;
		curr_edge < (nodes+current_index)->b_author_edges + n_author_edges; ++curr_edge)
		{
			int index = *(curr_edge);
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
		size_t n_publisher_edges = (nodes+current_index)->n_publisher_edges;
		for(curr_edge = (nodes+current_index)->b_publisher_edges;
		curr_edge < (nodes+current_index)->b_publisher_edges + n_publisher_edges; ++curr_edge)
		{
			int index = *(curr_edge);
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
		size_t n_citation_edges = (nodes+current_index)->n_citation_edges;
		for(curr_edge = (nodes+current_index)->b_citation_edges;
		curr_edge < (nodes+current_index)->b_citation_edges + n_citation_edges; ++curr_edge)
		{
			int index = *(curr_edge);
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
	}
	free(queue);
	// Return empty set if there was no path
	if(prev[b2_index] == -1)
		return result;
	// Backtrack to find the shortest path
	// *** Backtracking Algorithm ***
	int mcount = 1;
	size_t size = 10;
	book_t** elements = (book_t**)malloc(sizeof(book_t*)*size);
	elements[0] = (nodes+b2_index);
	while(prev[b2_index] != -1)
	{
		mcount++;
		b2_index = prev[b2_index];
		if(size < mcount)
		{
			size = size*2;
			elements = realloc(elements, sizeof(book_t*)*size);
		}
		elements[mcount-1] = (nodes+b2_index);
	}
	result->elements = realloc(result->elements, sizeof(book_t*)*mcount);
	int m = 0;
	#pragma unroll
	for(m = 0; m < mcount; ++m)
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
