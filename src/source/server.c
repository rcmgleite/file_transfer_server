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
#include <arpa/inet.h>		//usado para criar endereços de internet
#include <fcntl.h>
#include <pthread.h>

#include "server.h"
#include "server_conn_utils.h"
#include "server_utils.h"
#include <sys/resource.h>

pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;

int main(){
	int listener_d;			//socket que vai esperar pelas requests
	int fd;					//file descriptors que serão usados.. quando o cliente for escrito, o to_write será apagado
	char read_buffer[255];  //buffer usado para armazenar dados que vem do cliente
	int number_of_threads;
	int file_size;
	int curr_offset = 0;
	/**
	 *	setrlimit está sendo usado para aumentar o número limite de file descriptors
	 *  que este processo pode abrir
	 **/
	struct rlimit rlp;
	getrlimit(RLIMIT_NOFILE, &rlp);
	fprintf(stderr, "max fds before: %d %d\n", (int)rlp.rlim_cur, (int)rlp.rlim_max);
	rlp.rlim_cur = 4000;
	setrlimit(RLIMIT_NOFILE, &rlp);

	getrlimit(RLIMIT_NOFILE, &rlp);
	fprintf(stderr, "max fds after: %d %d\n", (int)rlp.rlim_cur, (int)rlp.rlim_max);

	//contador
	int i;

	open_listener(&listener_d);
	bind_to_port(listener_d, SERVER_PORT, 1);

	if( listen(listener_d, 10) == -1){ //escuto o socket para conexoes e coloco o 10 para aceitar no max 10 cliente tentando conexão
		fprintf(stderr, "Unable to initialize listen operation... Shutting down!\n\n");
		exit(1);
	}

	fprintf(stdout, "Waiting for connections...\n");

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
			fprintf(stderr, "VAI BLOCAR NO RECV\n");
			read_from_client(connect_d, read_buffer, 255);
			/**
			 *	Tentativa de abrir arquivo fonte
			 **/
			char *file_path = build_file_path(read_buffer, 0);
			fd = open(file_path, O_RDONLY);
			free(file_path);

			if(fd == -1){
				fprintf(stderr, "Unable to open file!\n\n");
				write_to_client(connect_d, "Unable to open file!\n", strlen("Unable to open file!\n"));
			}
			else{
				file_size = lseek(fd, 0L, SEEK_END);	//Vou ao final do arquivo para poder saber o tamanho dele
				lseek(fd, 0L, SEEK_SET);	//Volto para o início do arquivo para poder começar a operação

				fprintf(stdout, "file_size: %d\n", file_size);
				/**
				 *	Cálculo do número de threads necessárias usando o tamanho total do arquivo
				 **/
				number_of_threads = get_numberof_threads(file_size);
				fprintf(stdout, "CALCULATED NUMBER OF THREADS: %d\n", number_of_threads);

				/**
				 *	HEADER PARA QUE O CLIENTE SAIBA QUANTAS THREADS SERÃO NECESSÁRIAS E O TAMANHO DO ARQUIVO
				 **/
				print_header(connect_d, number_of_threads, file_size);

				/**
				 *	Alocação de memória para threads e para a estrutura de argumentos
				 **/
				pthread_t *threads;
				threads = (pthread_t *)malloc(number_of_threads * sizeof(*threads));
				struct thread_args *args;
				args = (struct thread_args *)malloc(number_of_threads * sizeof(*args));

				/**
				 *	Inicialização das threads
				 **/
				for(i = 0; i < number_of_threads; i++){
					initialize_thread(&threads[i], &args[i], i, connect_d, fd, &file_size, &curr_offset);
				}

				//Apenas para o programa esperar as threads executarem
			    for (i = 0; i < number_of_threads; i++){
			        pthread_join(threads[i], NULL);
			    }

			    // Limpo todos os dados para a próxima requesição
			    clean_up(fd, threads, args, &number_of_threads, &file_size, &curr_offset);
			}
		}else{
			fprintf(stderr, "Erro ao escrever mensagem para o cliente");
		}
		close(connect_d);
	}

	return 0;
}

void *thread_function(void *args){

	/**
	 * 	Abrindo os sockets para as conexões TCP entre threads
	 **/
	int new_conn;
	open_listener(&new_conn);
	/*Colocando SERVER_PORT + args->thread_number vamos começar no 30000 e subir até o quanto precisarmos*/
	bind_to_port(new_conn, SERVER_PORT + 1 + ((_thread_args*)args)->thread_number, 1);

	if( listen(new_conn, 10) == -1){ /*no max 1 cliente tentando conexão*/
		fprintf(stderr, "Unable to initialize listen operation... Shutting down!\n\n");
		exit(1);
	}

	struct sockaddr_storage client_addr;
	unsigned int address_size = sizeof(client_addr);
	int transf_sock = accept(new_conn, (struct sockaddr*)&client_addr, &address_size);
	if(transf_sock == -1){
		fprintf(stderr, "\n\nNão foi possível abrir o segundo socket da THREAD %d\n\n", ((_thread_args*)args)->thread_number);
	}

	pthread_mutex_lock(&_lock);

	char *file_segment;
	file_segment = malloc(((_thread_args*)args)->chunk_size * sizeof(*file_segment));
	int bytes_read;
	fprintf(stdout, "\nSERÃO LIDOS: %d\n", ((_thread_args*)args)->chunk_size);
	fprintf(stdout, "THREAD NUMBER: %d\n", ((_thread_args*)args)->thread_number);
	lseek(((_thread_args*)args)->fd, ((_thread_args*)args)->file_offset, SEEK_SET);

	/**
	 *	O cliente precisa do offset, do tamanho que terá que escrever(para ler) e do segment
	 **/
	char c_offset[30], c_chunk_size[30];
	sprintf(c_offset, "%d\n", ((_thread_args*)args)->file_offset);
	sprintf(c_chunk_size, "%d\n", ((_thread_args*)args)->chunk_size);

	send(transf_sock, c_offset, strlen(c_offset), 0);
	send(transf_sock, c_chunk_size, strlen(c_chunk_size), 0);

	while(((_thread_args*)args)->chunk_size != 0){
		bytes_read = read(((_thread_args*)args)->fd, file_segment, 2048);
		if(bytes_read < 0)
			fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");

		send(transf_sock, file_segment, bytes_read, 0);
		((_thread_args*)args)->chunk_size -= bytes_read;
	}

	free(file_segment);

	/**
	 *	Fecha conexão TCP ao final da transmissão
	 **/
	close(new_conn);
	pthread_mutex_unlock(&_lock);
	return NULL;
}

void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int client_sock, int fd, int *file_size, int *curr_offset){
	args->thread_number = thread_number;
	args->client_sock = client_sock;
	args->fd = fd;
	//parte da incialização que muda para cada thread

	if(*file_size - FIRST_GUESS_OFFSET > 0){
		//ainda está no ponto onde as threads escrevem exatamente o tamanho do chunk
		args->file_offset = *curr_offset;
		args->chunk_size = FIRST_GUESS_OFFSET;
		*file_size -= args->chunk_size;
		*curr_offset +=args->chunk_size;
	}
	else{
		//aqui já teremos um valor menor de chunk (última parte do arquivo)
		args->file_offset = *curr_offset;
		args->chunk_size = *file_size;
		*file_size -= args->chunk_size;
		*curr_offset +=args->chunk_size;
	}
	pthread_create(thread, NULL, thread_function, (void*)args);
}

int get_numberof_threads(int file_size){
	return (file_size / FIRST_GUESS_OFFSET + 1);
}

void clean_up(int fd, pthread_t *threads, struct thread_args *args, int *number_of_threads,
		int *file_size, int *curr_offset){
    close(fd);
    free(threads);
    free(args);
	*number_of_threads = 0;
	*file_size = 0;
	*curr_offset = 0;
}
