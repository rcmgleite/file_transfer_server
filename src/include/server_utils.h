/*
 * server_utils.h
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */

#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include <sys/socket.h>
#include <arpa/inet.h>		//usado para criar endereços de internet
#include <fcntl.h>

#define PROGRAM_NAME "proj_redes_server"
#define MAX_WRITE_SIZE 10240

typedef struct thread_args{
	int client_sock;
	long file_offset;
	long chunk_size;
	int thread_number;
	char *file_path;
}_thread_args;

void build_args(struct thread_args *args, int thread_number, int client_sock, long *file_size, long *curr_offset, char *file_path);
void* thread_function(void *args);

char* format_file_path(char* file_name);
int read_from_client(int sock, char *buf, int len);
int write_to_client(int sock, char *buf, int length);
void print_header(int sock, int number_of_threads, long file_size);
void print_init_transmission(int sock);
#endif /* SERVER_UTILS_H_ */
