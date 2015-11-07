/*
 * server.c
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "server.h"
#include "server_conn_utils.h"
#include "server_utils.h"
#include "thread_pool.h"
#include "utils.h"

int main(int argc, char *argv[]){
	int listener_d;			//Server Socket
	int fd;
	char read_buffer[255];
	long file_size;
	long curr_offset = 0;

	DEBUG("Initializing thread pool");
	thread_pool_t* pool = new_thread_pool(N_THREADS);
	DEBUG("Done!");

	open_listener(&listener_d);
	bind_to_port(listener_d, SERVER_PORT, 1);

	if( listen(listener_d, MAX_CONN) == -1){
		fprintf(stderr, "Unable to initialize listen operation... Shutting down!\n\n");
		exit(1);
	}

	DEBUG("Waiting for connections...");

	while(TRUE){
		/**
		 * struct sockaddr_storage that is designed to be large enough to hold both IPv4 and IPv6 structures.
		 * (See, for some calls, sometimes you don't know in advance if it's going to fill out your struct sockaddr
		 * with an IPv4 or IPv6 address. So you pass in this parallel structure, very similar to struct
		 * sockaddr except larger, and then cast it to the type you need
		 **/
		struct sockaddr_storage client_addr;
		unsigned int address_size = sizeof(client_addr);
		int connect_d = accept(listener_d, (struct sockaddr*)&client_addr, &address_size);
		if(connect_d == -1){
			fprintf(stderr, "\n\nNão foi possível abrir o segundo socket\n\n");
		}
		fprintf(stderr, "Abriu o socket de conexão com o cliente\n");

		if(write_to_client(connect_d, "Digite o nome do aquivo desejado\n", strlen("Digite o nome do aquivo desejado\n")) != -1){
			read_from_client(connect_d, read_buffer, 255);

			char *file_path = format_file_path(read_buffer);

			fprintf(stdout, "file_path: %s\n", file_path);
			fd = open(file_path, O_RDWR);

			if(fd == -1){
				fprintf(stderr, "Unable to open file!\n\n");
				write_to_client(connect_d, "Unable to open file!\n", strlen("Unable to open file!\n"));
			}
			else{
				file_size = lseek(fd, 0L, SEEK_END);	//Vou ao final do arquivo para poder saber o tamanho dele

				close(fd);
				fprintf(stdout, "file_size: %lu\n", file_size);

				/**
				 *	Header - n_threads and file size are sent to client
				 **/
				print_header(connect_d, N_THREADS, file_size);

				struct thread_args *args;
				args = (struct thread_args *)malloc(N_THREADS * sizeof(*args));

				struct timeval tvalBefore, tvalAfter;  // removed comma
				gettimeofday (&tvalBefore, NULL);

				for(int i = 0; i < N_THREADS; i++){
					build_args(&args[i], i, connect_d, &file_size, &curr_offset, file_path);
				}

				print_init_transmission(connect_d);

				for(int i = 0; i < N_THREADS; i++) {
					pool_add_job(pool, thread_function, (void*) (&args[i]));
				}

				pool_wait_finish(pool);

			    gettimeofday (&tvalAfter, NULL);
			    fprintf(stderr, "Aqruivo transferido!\n");

				printf("Time Elapsed: %f sec\n",
							                ((tvalAfter.tv_sec - tvalBefore.tv_sec)
							               + (tvalAfter.tv_usec - tvalBefore.tv_usec)/(float)1000000)
							              );;
			}
		}else{
			fprintf(stderr, "Erro ao escrever mensagem para o cliente");
		}
		close(connect_d);
	}

	return 0;
}




