/*
 * server.h
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */

#ifndef SERVER_H_
#define SERVER_H_
#include <pthread.h>		//lib das threads

#define TRUE 1
#define FALSE 1

/*
*	Usando FIRST GUESS OFFSET calculo o número de threads necessárias para a execução da transferência
**/

//#define FIRST_GUESS_OFFSET 4194304
//#define FIRST_GUESS_OFFSET 243184258		//single thread - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 134217728		//2 thread - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 67108864		//4 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 33554432		//8 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 16777216		//15 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 8388608			//29 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 243184257
#define FIRST_GUESS_OFFSET 2928660482		//single thread - 3G
//#define FIRST_GUESS_OFFSET	1464330242		// 2 threads - 3G
//#define FIRST_GUESS_OFFSET 1073741824		//3 threads - 3G
//#define FIRST_GUESS_OFFSET 536870912		//6 threads - 3G
//#define FIRST_GUESS_OFFSET 268435456		//11 threads - 3G

void *thread_function(void *args);
int get_numberof_threads(long file_size);

typedef struct thread_args{
	int client_sock;
	long file_offset;
	long chunk_size;
	int thread_number;
	char *file_path;
}_thread_args;
void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int client_sock, long *file_size, long *curr_offset, char *file_path);
void clean_up(pthread_t *threads, struct thread_args *args, int *number_of_threads,
		long *file_size, long *curr_offset, char *file_path);
#endif /* SERVER_H_ */
