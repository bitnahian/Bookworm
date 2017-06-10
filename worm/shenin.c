int id_found_book = 0;

void* worker(void * arg)
{
	tdata* thdata = (tdata*) arg;

	int chunk =  thdata->count / g_nthreads;
	int remainder = thdata->count % g_nthreads;
	int start = chunk * thdata->thread_id;
	int end = chunk + start;

	if(end == thdata->count - remainder)
		end += remainder;

	for(int i = start; i < end; ++i)
	{
 		printf("%zu\n", thdata->nodes[i].id);
 		printf("%zu\n", thdata->book_id);

		//Checking to see if book_id = node->id
		if(thdata->nodes[i].id == thdata->book_id)
		{
			printf("HERE\n");

			pthread_mutex_lock(&my_lock);

			//Found is true. All other threads will stop executing this if loop
			found = true;

			pthread_mutex_unlock(&my_lock);

			thdata->found_book = &thdata->nodes[i];

			// printf("1: %zu\n", thdata->found_book->id);
			// printf("2: %zu\n", thdata->nodes->id);

			id_found_book = thdata->thread_id;

			break;
		}


		//This condition is for the other threads so that they can
		//break away from the loop once it has been found.
		if(found == true)
		{
			printf("Not found\n\n");
			break;
		}

	}

	return thdata;
}


// Returns result set containing book with given id.
result_t* find_book(book_t* nodes, size_t count, size_t book_id)
{
	//Making 4 threads
	pthread_t my_threads[g_nthreads];

	//Keeps track of how many threads created
	int created = 0;

	//mallocing the result_t struct to store the found book in it and
	//initialising the struct size
	result_t* result = (result_t*)malloc(sizeof(result_t));

	result->elements = (book_t**)malloc(sizeof(book_t*));

	result->n_elements = 0;

	tdata* data[g_nthreads];

	for(int i = 0; i< g_nthreads; ++i)
	{
		data[i] = (tdata*)malloc(sizeof(tdata));

		tdata* data_th = data[i];

		//assigning to data_th
		data_th->thread_id = i;
		data_th->nodes = nodes;
		data_th->book_id = book_id;
		data_th->count = count;
		data_th->found_book = malloc(sizeof(book_t));

		created += !pthread_create(my_threads + i, NULL, worker, data_th);
	}

	for(int i = 0; i < created; ++i)
	{
		tdata *final_data;
		pthread_join(my_threads[i], (void**)&final_data);

		final_data = data[i];

		if(final_data != NULL)
		{
		//	printf("%zu\n", data[id_found_book]->found_book->id);

			result->elements[0] = data[i]->found_book;

			printf("RESULT ID: %zu\n\n", result->elements[0]->id);

			result->n_elements = 1;
			continue;
		}

		free(data[i]->found_book);
		free(data[i]);

	}

	found = false;
	return result;
}
