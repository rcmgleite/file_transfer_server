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

char *build_file_path(char* file_name, int is_new){
	size_t i, slen = strlen(file_name);
	for (i = 0; i < slen; i++) {
		if(file_name[i] == '\r')
			file_name[i] = '\0';
	}

	char *file_path;
	if(!is_new){
		file_path = malloc(sizeof(ROOT_PATH) + sizeof(file_name));
		strcpy(file_path, ROOT_PATH);
	}
	else{
		file_path = malloc(sizeof(NEW_ROOT_PATH) + sizeof(file_name));
		strcpy(file_path, NEW_ROOT_PATH);
	}
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
//	int slen = len;
	int c = recv(sock, s, len, 0);
	fprintf(stderr, "LEU ALGUMA PARADA: %s\n", s);
//	while((c > 0) && (s[c - 1] != '\n')){
//		fprintf(stderr, "Entrou no while do read_from_client\n");
//		s += c;
//		slen -= c;
//		c = recv(sock, s, slen, 0);
//	}
//	if(c < 0){
//		return c;
//	}
//	else if(c == 0){
//		buf[0] = '\0';
//	}
//	else{
//		s[c-1] = '\0';
//	}
	return c;
}


void print_header(int sock, int number_of_threads, int file_size){
	char n_threads_string[10], file_size_string[10];
	sprintf(n_threads_string, "%d\n", number_of_threads);
	sprintf(file_size_string, "%d\n", file_size);
	char bb[] = "\n\n";
	int size = strlen(n_threads_string) + strlen(file_size_string) + strlen(bb);
	char *header = malloc(size*sizeof(char));
	strcat(header, n_threads_string);
	strcat(header, file_size_string);
	strcat(header, bb);
	write_to_client(sock, header, size);
}
