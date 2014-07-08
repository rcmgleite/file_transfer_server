/*
 * web_server_conn_utils.c
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */
#include "server_conn_utils.h"
#include <sys/socket.h>		//HEADER PARA TER ACESSO À STRUCT DO SOCKET
#include <arpa/inet.h>		//usado para criar endereços de internet
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void open_listener(int *listener){
	int _socket = socket(PF_INET, SOCK_STREAM, 0);
	if(_socket == -1){
		fprintf(stderr, "Unable to open listener socket!\nTerminating program...");
		fprintf(stderr, "%d", errno);
		exit(1);
	}
	fprintf(stderr, "ABRIU O SOCKET COM SUCESSO\n");
	*listener = _socket;
}

void bind_to_port(int _socket, int _port, int reuse){
	//Essa parte do código é apenas para que a porta possa ser reusada caso matarmos os server
	//e tentarmos subir ele novamente muito rápido.
	if(setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1){
		fprintf(stderr, "Unable to set reuse on socket!\n");
	}

	struct sockaddr_in name;
	name.sin_family = PF_INET;
	name.sin_port = (in_port_t)htons(_port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);				//qual o tipo de ip que pode ser usado(apenas um filtro)
	int my_bind = bind(_socket, (struct sockaddr *) &name, sizeof(name)); //bind do socket para a porta
	if(my_bind == -1){
		fprintf(stderr, "port: %d\n", _port);
		fprintf(stderr, "Unable to bind socket... Shutting down!\n");
		exit(1);
	}
}


