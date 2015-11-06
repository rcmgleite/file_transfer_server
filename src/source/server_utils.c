/*
 * server_utils.c
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server_utils.h"

void build_args(struct thread_args *args, int thread_number, int client_sock, long *file_size, long *curr_offset, char *file_path){
	args->thread_number = thread_number;
	args->client_sock = client_sock;
	args->file_path = file_path;

	if(*file_size - MAX_WRITE_SIZE > 0){
		args->file_offset = *curr_offset;
		args->chunk_size = MAX_WRITE_SIZE;
		*file_size -= args->chunk_size;
		*curr_offset +=args->chunk_size;
	}
	else{
		args->file_offset = *curr_offset;
		args->chunk_size = *file_size;
		*file_size -= args->chunk_size;
		*curr_offset +=args->chunk_size;
	}
}

void* thread_function(void *args){
	int fd_read = open(((_thread_args*)args)->file_path, O_RDWR);

	char *file_segment;
	file_segment = malloc(((_thread_args*)args)->chunk_size * sizeof(*file_segment));
	long bytes_read;
	lseek(fd_read, ((_thread_args*)args)->file_offset, SEEK_SET);

	char c_offset[30], c_chunk_size[30];
	sprintf(c_offset, "%lu\n", ((_thread_args*)args)->file_offset);
	sprintf(c_chunk_size, "%lu\n", ((_thread_args*)args)->chunk_size);

	write(((_thread_args*)args)->client_sock, c_offset, strlen(c_offset));
	write(((_thread_args*)args)->client_sock, c_chunk_size, strlen(c_chunk_size));

	long write_size;

	if(((_thread_args*)args)->chunk_size <= MAX_WRITE_SIZE){
		write_size = ((_thread_args*)args)->chunk_size;
	}
	else{
		write_size = MAX_WRITE_SIZE;
	}

	while(((_thread_args*)args)->chunk_size != 0){
		bytes_read = read(fd_read, file_segment, write_size);
		if(bytes_read < 0)
			fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");

		write(((_thread_args*)args)->client_sock, file_segment, bytes_read);
		((_thread_args*)args)->chunk_size -= bytes_read;
		if(write_size >= ((_thread_args*)args)->chunk_size){
			write_size = ((_thread_args*)args)->chunk_size;
		}
	}

	free(file_segment);
	return NULL;
}

char* format_file_path(char* file_name){
	size_t i, slen = strlen(file_name);
	for (i = 0; i < slen; i++) {
		if(file_name[i] == '\r')
			file_name[i] = '\0';
	}
	return file_name;
}

int write_to_client(int sock, char *buf, int length){
	int result = send(sock, buf, length, 0);
	if(result == -1){
		fprintf(stderr, "\n\nErro ao tentar escrever para o cliente\n\n");
	}
	return result;
}

int read_from_client(int sock, char *buf, int len){
	char *s = buf;
	int c = recv(sock, s, len, 0);
	return c;
}


void print_header(int sock, int number_of_threads, long file_size){
	char n_threads_string[255], file_size_string[255];
	sprintf(n_threads_string, "%d\n", number_of_threads);
	sprintf(file_size_string, "%lu\n", file_size);
	int size = strlen(n_threads_string) + strlen(file_size_string);
	char *header = malloc(size*sizeof(char));
	strcat(header, n_threads_string);
	strcat(header, file_size_string);
	write_to_client(sock, header, size);
	fprintf(stderr, "%s\n", header);
	free(header);
}

void print_init_transmission(int sock){
	char init[] = "init";
	write_to_client(sock, init, strlen(init));
}
