#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "worm.h"

#define MAX_BUFFER 65535

// Reads given graph file and returns a book graph.
book_t* graph_loader(size_t* count, char* filename) {

	char buffer[MAX_BUFFER];
	size_t n_books = 0;

	// Open graph file
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		perror("Fatal error! Unable to open graph file");
		return NULL;
	}

	// Read book count
	if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &n_books) == 0) {
		fprintf(stderr, "Fatal error! Unable to read book count.\n");
		return NULL;
	}

	book_t* graph = malloc(sizeof(book_t) * n_books);

	// Read books
	for (size_t i = 0; i < n_books; i++) {

		// Read book ID
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].id) == 0) {
			fprintf(stderr, "Fatal error! Unable to read book ID for book %zu.\n", i);
			return NULL;
		}

		// Read publisher ID
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].publisher_id) == 0) {
			fprintf(stderr, "Fatal error! Unable to read publisher ID for book %zu.\n", i);
			return NULL;
		}

		// Read author ID
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].author_id) == 0) {
			fprintf(stderr, "Fatal error! Unable to read author ID for book %zu.\n", i);
			return NULL;
		}

		size_t cap = 10;
		size_t size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		size_t length = MAX_BUFFER - 1;
		graph[i].b_publisher_edges = malloc(sizeof(size_t) * cap);

		// Read publisher edges
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to read publishers for book %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					cap = cap * 2;
					graph[i].b_publisher_edges = realloc(graph[i].b_publisher_edges, sizeof(size_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					size_t k = strtol(s, NULL, 10);
					graph[i].b_publisher_edges[size] = k;
					size++;
				}
			}
		}
		graph[i].n_publisher_edges = size;

		cap = 10;
		size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		length = MAX_BUFFER - 1;
		graph[i].b_author_edges = malloc(sizeof(size_t) * cap);

		// Read author edges
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to read publishers for book %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					cap = cap * 2;
					graph[i].b_author_edges = realloc(graph[i].b_author_edges, sizeof(size_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					size_t k = strtol(s, NULL, 10);
					graph[i].b_author_edges[size] = k;
					size++;
				}
			}
		}
		graph[i].n_author_edges = size;

		cap = 10;
		size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		length = MAX_BUFFER - 1;
		graph[i].b_citation_edges = malloc(sizeof(size_t) * cap);

		// Read citation edges
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to read citations for book %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					graph[i].b_citation_edges = realloc(graph[i].b_citation_edges, sizeof(uint32_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					uint32_t k = strtol(s, NULL, 10);
					graph[i].b_citation_edges[size] = k;
					size++;
				}
			}
		}
		graph[i].n_citation_edges = size;
	}

	*count = n_books;
	return graph;
}

void test_sample(book_t* graph, size_t count) {

	clock_t begin = clock();
	result_t* r1 = find_shortest_distance(graph, count, 41592, 2247);
	clock_t end = clock();

	printf("\n\n %lf seconds\n\n", (double)(end - begin) / CLOCKS_PER_SEC);

	free(r1->elements);
	free(r1);
/*
	result_t* r2 = find_books_by_author(graph, count, 1);
	if (r2 == NULL) {
		fprintf(stderr, "Fail! find_books_by_author() => result set is NULL.\n");
	} else if (r2->n_elements != 5) {
		fprintf(stderr, "Fail! find_books_by_author() => result set contains %zu elements.\n", r2->n_elements);
	}

	result_t* r3 = find_shortest_distance(graph, count, 5, 13);
	printf("%zu\n\n", r3->n_elements);
	for(int i = 0; i < r3->n_elements; ++i)
	{
		printf("%zu\n", r3->elements[i]->id);
	}

	result_t* r4 = find_books_reprinted(graph, count, 8);
	result_t* r5 = find_books_k_distance(graph, count, 9, 3);
	*/
	/*
	for(int i = 0; i  < r5->n_elements; ++i)
	{
		printf("book_index: %zu book_id: %zu\n", r5->elements[i]->index, r5->elements[i]->id);
	}
	*/
	/*
	free(r1->elements);
	free(r1);
	free(r2->elements);
	free(r2);
	free(r4->elements);
	free(r4);
	free(r5->elements);
	free(r5);

	free(r3->elements);
	free(r3);
	*/


}

int main(int argc, char** argv) {

	// Example usage

	size_t count = 0;
	book_t* graph = graph_loader(&count, "LargeLib.graph.graph");
	if (graph == NULL) {
		return 1;
	}

	test_sample(graph, count);

	for (size_t i = 0; i < count; i++) {
		free(graph[i].b_author_edges);
        free(graph[i].b_citation_edges);
		free(graph[i].b_publisher_edges);
	}

	free(graph);

	return 0;
}
