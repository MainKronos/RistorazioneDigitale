#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "header.h"

#define LEN_REPLY 6 

int main(int argc, char *argv[]){
	int ret, sd;
	struct sockaddr_in srv_addr;
	char buffer[1024];

	cmd command; /* Comando da inviare */

	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo del server */
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(1234);
	inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

	ret = connect(sd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

	if (ret < 0){
		perror("Errore in fase di connessione: \n");
		exit(-1);
	}


	strcpy(command, "ping");
	ret = send(sd, (void *)command, sizeof(command), 0);

	ret = recv(sd, command, sizeof(command), 0);

	printf("%.*s",
		(int)sizeof(command), 
		command
	);

	if (ret < 0){
		perror("Errore in fase di ricezione: \n");
		exit(-1);
	}
	sleep(1);


	close(sd);
}