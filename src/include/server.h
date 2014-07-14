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
#define FIRST_GUESS_OFFSET 243184258		//single thread - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 134217728		//2 thread - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 67108864		//4 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 33554432		//8 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 16777216		//15 threads - Supernatural.S08E21.HDTV.x264-LOL.mp4
//#define FIRST_GUESS_OFFSET 243184257

void *thread_function(void *args);
int get_numberof_threads(int file_size);

typedef struct thread_args{
	int client_sock;
	int file_offset;
	int chunk_size;
	int thread_number;
	char *file_path;
}_thread_args;
void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int client_sock, int *file_size, int *curr_offset, char *file_path);
void clean_up(int fd, pthread_t *threads, struct thread_args *args, int *number_of_threads,
		int *file_size, int *curr_offset, char *file_path);
#endif /* SERVER_H_ */
