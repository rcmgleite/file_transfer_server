/*
 * thread_pool.h
 *
 *  Created on: Nov 5, 2015
 *      Author: rafael
 */

#ifndef SRC_INCLUDE_THREAD_POOL_H_
#define SRC_INCLUDE_THREAD_POOL_H_

#define MAX_THREADS 100

/*
 *	API;
 */
typedef struct thread_pool_t thread_pool_t;
thread_pool_t* new_thread_pool(unsigned n_threads);
int pool_add_job(thread_pool_t* pool, void* (*func) (void*), void* arg);

#endif /* SRC_INCLUDE_THREAD_POOL_H_ */
