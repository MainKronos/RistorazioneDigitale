#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "header.h"

void job(int socket, fd_set* master){
	struct sockaddr_in cl_addr; /* Indirizzo client */
	int nbytes; /* Numero di byte letti */
	cmd command; /* Comando da processare */

	/* Gestione dati in arrivo da un client */
	nbytes = recv(socket, command, sizeof(command), 0);
	if (nbytes < 0){
		perror("Errore in fase di recv: \n");
		exit(-1);
	}
	if(nbytes == 0) {
		/* Connessione chiusa */
		printf("Connessione chiusa da %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
		FD_CLR(socket, master);
		close(socket);
	} else {
		/* Gestione dati */
		printf("Comando %.*s da %s:%d\n",
			(int)sizeof(command), 
			command,
			inet_ntoa(cl_addr.sin_addr),
			ntohs(cl_addr.sin_port)
		);

		if(strcmp(command, "ping") == 0) {
			strcpy(command, "pong");
			nbytes = send(socket, command, sizeof(command), 0);
			if (nbytes < 0){
				perror("Errore in fase di send: \n");
				exit(-1);
			}	
		}
	}
}

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	fd_set master; /* Set principale gestito dal programmatore con le macro */
	fd_set read_fds; /* Set di lettura gestito dalla select */
	int fdmax; /* Numero max di descrittori */
	struct sockaddr_in sv_addr; /* Indirizzo server */
	struct sockaddr_in cl_addr; /* Indirizzo client */
	int listener; /* Socket per l'ascolto */
	int newfd; /* Nuova connessione */
	
	socklen_t addrlen;
	int i;
	int ret;
	
	/* ---------------------------------------------------------------------------- */

	/* Controllo comando*/
	if(argc != 2) {
		fprintf(stderr, "Usage: /server <porta>");
		exit(-1);
	}

	/* --- Setup ------------------------------------------------------------------- */
	/* Azzero i set */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	/* Creazione socket */
	listener = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo */
	sv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);
	sv_addr.sin_port = htons(atoi(argv[1]));

	/* Aggancio del socket all'indirizzo */
	ret = bind(listener, (struct sockaddr *)&sv_addr, sizeof(sv_addr));
	if (ret < 0){
		perror("Errore in fase di bind: \n");
		exit(-1);
	}

	/* Inizio dell'ascolto, coda da 10 connessioni */
	ret = listen(listener, 10);
	if (ret < 0){
		perror("Errore in fase di listen: \n");
		exit(-1);
	}

	/* Aggiungo il listener al set principale */
	FD_SET(listener, &master);

	/* Aggiorno il valore di fdmax */
	fdmax = listener;
	/* ---------------------------------------------------------------------------- */

	/* --- Ciclo principale ------------------------------------------------------- */
	while(1) {
		read_fds = master; /* Copio il set principale */
		ret = select(fdmax+1, &read_fds, NULL, NULL, NULL);
		if (ret < 0){
			perror("Errore in fase di select: \n");
			exit(-1);
		}

		/* Scorro i descrittori */
		for(i=0; i<=fdmax; i++) {
			if(FD_ISSET(i, &read_fds)) {
				if(i == listener) {

					/* Gestione nuova connessione */
					addrlen = sizeof(cl_addr);
					newfd = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
					if (newfd < 0){
						perror("Errore in fase di accept: \n");
						exit(-1);
					}
					FD_SET(newfd, &master);
					if(newfd > fdmax) {
						fdmax = newfd;
					}
					printf("Nuova connessione da %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
				} else {
					
					job(i, &master);
				}
			}
		}
	}
	/* ---------------------------------------------------------------------------- */

	return 0;

	
}