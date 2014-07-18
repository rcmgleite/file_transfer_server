/*
 * server_utils.c
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "server_utils.h"

char *build_file_path(char* file_name, char *dir_path){
	size_t i, slen = strlen(file_name);
	for (i = 0; i < slen; i++) {
		if(file_name[i] == '\r')
			file_name[i] = '\0';
	}
	slen = strlen(dir_path);
	int prog_name_size = strlen(PROGRAM_NAME);
	char *corrected_dir_path = (char*) malloc((slen + 1 - prog_name_size) * sizeof(char*));

	i = 0;
	while(i < slen - strlen(PROGRAM_NAME)){
		corrected_dir_path[i] = dir_path[i];
		i++;
	}

	char *file_path;
	file_path = malloc(strlen(corrected_dir_path) + strlen(file_name));
	strcpy(file_path, corrected_dir_path);
	strcat(file_path, file_name);
	return file_path;
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
