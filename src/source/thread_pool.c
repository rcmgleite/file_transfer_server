/*
 * thread_pool.c
 *
 *  Created on: Nov 6, 2015
 *      Author: rafael
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "thread_pool.h"
#include "utils.h"

/*
 *	Struct that represents the thread pool task.
 */
typedef struct job_t {
	void* (*func) (void*);
	void* arg;
	struct job_t* next;
} job_t;

/*
 *	Struct that represents the job queue for the pool
 */
typedef struct job_queue_t {
	job_t* first;
	job_t* last;
	unsigned length;
} job_queue_t;

/*
 * 	Wrapper to pthread just to add the id field
 */
typedef struct thread_t {
	unsigned id;
	pthread_t *pthread;
} thread_t;

/*
 *	Struct that represents the complete thread pool
 */
typedef struct thread_pool_t {
	thread_t* threads;
	job_queue_t queue;
	pthread_cond_t has_jobs;
	volatile unsigned n_threads_blocked;
	volatile unsigned n_threads_working;
	unsigned n_threads;
	pthread_mutex_t mutex;
} thread_pool_t;

/*
 *	Queue Prototypes
 */
static void job_queue_init(job_queue_t* queue);
static job_t* next_job(job_queue_t* queue);
static void queue_add_job(job_queue_t* queue, job_t* job);

/*
 *	Thread Prototypes
 */
static void *thread_func(void *args);
static void alloc_threads(thread_pool_t* pool, unsigned n_threads);


/*
 *	Returns a new thread pool
 */
thread_pool_t* new_thread_pool(unsigned n_threads) {
	if(n_threads < 0 || n_threads > MAX_THREADS) {
		fprintf(stderr, "[ERROR] Number of threads out of bounds");
		return NULL;
	}

	thread_pool_t* pool = (thread_pool_t*) malloc(sizeof(thread_pool_t));
	if(pool == NULL) {
		fprintf(stderr, "[ERROR] Unable to allocate memory for thread pool\n");
		return NULL;
	}

	pool->n_threads = n_threads;
	pool->n_threads_blocked = 0;
	pool->n_threads_working = 0;

	alloc_threads(pool, n_threads);
	/*
	 *	job_queue initialization
	 */
	job_queue_init(&pool->queue);
	/*
	 * 	mutex and condigiton variable initialization
	 */
	pthread_mutex_init(&pool->mutex, NULL);
	pthread_cond_init(&pool->has_jobs, NULL);

	/*
	 *	Start pool threads
	 */
	for(unsigned i = 0; i < pool->n_threads; i++) {
		pthread_create(pool->threads[i].pthread, NULL, thread_func, (void*) pool);
	}

	return pool;
}

/*
 *	Adds a new job on the thread pool to be executed
 */
int pool_add_job(thread_pool_t* pool, void* (*func) (void*), void* arg) {
	if(pool == NULL || func == NULL) {
		return 0;
	}

	job_t* job = (job_t*) malloc(sizeof(job_t));
	job->func = func;
	job->arg = arg;
	job->next = NULL;

	pthread_mutex_lock(&pool->mutex);

	queue_add_job(&pool->queue, job);
	pthread_cond_signal(&pool->has_jobs);

	pthread_mutex_unlock(&pool->mutex);
	return 1;
}

/*
 *	Initialize job queue
 */
static void job_queue_init(job_queue_t* queue) {
	queue = (job_queue_t*) malloc(sizeof(job_queue_t));
	queue->length = 0;
	queue->first = NULL;
	queue->last = NULL;
}

/*
 *	Returns next job on queue
 */
static job_t* next_job(job_queue_t* queue) {
	if(queue->length == 0) {
		fprintf(stderr, "[ERROR] No jobs on queue");
		return NULL;
	}

	job_t* _next_job = queue->first;
	queue->first = _next_job ->next;
	queue->length--;
	return _next_job;
}

/*
 *	Internal function that adds a new job to the pool queue
 */
static void queue_add_job(job_queue_t* queue, job_t* job) {
	if(queue->length == 0) {
			queue->first = queue->last = job;
	} else {
		queue->last->next = job;
		queue->last = job;
	}
	queue->length++;
}

/*
 *	Thread function
 *		Each thread on the pool will be running this function since creation
 *		The idea is that each one of them will be waiting for some job to be
 *		added to the queue. When that happens, one of them will acquire the job
 *		and execute it
 */
static void* thread_func(void *args) {
	thread_pool_t* pool =(thread_pool_t*) args;

	while(1) {
		pthread_mutex_lock(&pool->mutex);
		while(pool->queue.length == 0) {
			DEBUG("Wating for jobs...");
			pthread_cond_wait(&pool->has_jobs, &pool->mutex);
		}
		DEBUG("Got a Job!");

		job_t* job = next_job(&pool->queue);
		if(job == NULL)
			continue;

		pthread_mutex_unlock(&pool->mutex);
		job->func(job->arg);
	}

	return NULL;
}

/*
 *	Alloc new threads based on the thread_number
 */
static void alloc_threads(thread_pool_t* pool, unsigned n_threads) {
	pool->threads= (thread_t*) malloc(n_threads * sizeof(thread_t));
	for(unsigned i = 0; i < n_threads; i++) {
		pool->threads[i].id = i;
		pool->threads[i].pthread = (pthread_t*) malloc(sizeof(pthread_t));
	}
}

