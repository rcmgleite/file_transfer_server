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

pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;

int main(){
	int listener_d;			//socket que vai esperar pelas requests
	int fd, fd_to_write;	//file descriptors que serão usados.. quando o cliente for escrito, o to_write será apagado
	char read_buffer[255];  //buffer usado para armazenar dados que vem do cliente
	int number_of_threads;
	int file_size;
	int curr_offset = 0;

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
		struct sockaddr_storage client_addr;
		unsigned int address_size = sizeof(client_addr);
		int connect_d = accept(listener_d, (struct sockaddr*)&client_addr, &address_size);
		if(connect_d == -1){
			fprintf(stderr, "\n\nNão foi possível abrir o segundo socket\n\n");
		}
		fprintf(stderr, "Abriu o socket de conexão com o cliente\n");

		if(write_to_client(connect_d, "Digite o nome do aquivo desejado\n", sizeof("Digite o nome do aquivo desejado\n")) != -1){
			fprintf(stderr, "VAI BLOCAR NO RECV\n");
			read_from_client(connect_d, read_buffer, sizeof(read_buffer));
			fprintf(stdout, "LEU A PORRA DO NOME DO ARQUIVO DO CLIENTE\n");
			/**
			 *	Tentativa de abrir arquivo fonte
			 **/
			char *file_path = build_file_path(read_buffer);
			fd = open(file_path, O_RDONLY);

			/**
			 *	Abertura/Criação do arquivo destino
			 **/
			fd_to_write = open("/home/rafael/Desktop/rafael/C/proj_redes_server/Debug/teste_new.txt",O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);

			if(fd == -1 || fd_to_write == -1){
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
					initialize_thread(&threads[i], &args[i], i, connect_d, fd, &file_size, &curr_offset, fd_to_write);
				}

				//Apenas para o programa esperar as threads executarem
			    for (i = 0; i < number_of_threads; i++){
			        pthread_join(threads[i], NULL);
			    }

			    // Limpo todos os dados para a próxima requesição
			    clean_up(fd, fd_to_write, threads, args, &number_of_threads, &file_size, &curr_offset, file_path);
			}
		}else{
			fprintf(stderr, "Erro ao escrever mensagem para o cliente");
		}
		close(connect_d);
	}

	return 0;
}

void *thread_function(void *args){
	pthread_mutex_lock(&_lock);
	char *file_segment;
	file_segment = malloc(((_thread_args*)args)->chunk_size * sizeof(*file_segment));
	int bytes_read;
	fprintf(stdout, "\n\nSERÃO LIDOS: %d\n", ((_thread_args*)args)->chunk_size);
	fprintf(stdout, "\nTHREAD NUMBER: %d\n", ((_thread_args*)args)->thread_number);
	lseek(((_thread_args*)args)->fd, ((_thread_args*)args)->file_offset, SEEK_SET);
	bytes_read = read(((_thread_args*)args)->fd, file_segment, ((_thread_args*)args)->chunk_size);
	if(bytes_read < 0)
		fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");
	fprintf(stdout, "%s\n\n", file_segment);
	lseek(((_thread_args*)args)->fd_to_write, ((_thread_args*)args)->file_offset, SEEK_SET);
	write(((_thread_args*)args)->fd_to_write, file_segment, ((_thread_args*)args)->chunk_size);
	int result = write(((_thread_args*)args)->client_sock, file_segment, ((_thread_args*)args)->chunk_size);
	if(result == -1){
		fprintf(stderr, "\n\nErro ao tentar escrever para o cliente\n\n");
	}
	free(file_segment);
	pthread_mutex_unlock(&_lock);
	return NULL;
}

void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int client_sock, int fd, int *file_size, int *curr_offset, int fd_to_write){
	args->thread_number = thread_number;
	args->client_sock = client_sock;
	args->fd = fd;
	args->fd_to_write = fd_to_write;
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

void clean_up(int fd, int fd_to_write, pthread_t *threads, struct thread_args *args, int *number_of_threads,
		int *file_size, int *curr_offset, char *file_path){
    close(fd);
    close(fd_to_write);
    free(threads);
    free(args);
    free(file_path);
	*number_of_threads = 0;
	*file_size = 0;
	*curr_offset = 0;
}
