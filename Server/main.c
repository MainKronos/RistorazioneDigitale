#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "header.h"

#include "server.h" /* Variabili globali */

/* Gestore Socket */
void* socketHandler(void* arg);

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	fd_set master; /* Set principale */
	fd_set read_fds; /* Set di lettura gestito dalla select */
	int fdmax; /* Numero max di descrittori */
	struct sockaddr_in sv_addr; /* Indirizzo server */
	struct sockaddr_in cl_addr; /* Indirizzo client */
	pthread_t thread; /* Thread per la gestione della connessione */
	int listener; /* Socket per l'ascolto */
	int* sock; /* Socket nuova connessione */
	int nbytes; /* Numero di byte letti */
	
	cmd command; /* Comando da utente processare */

	socklen_t addrlen;
	int ret; /* Valore di ritorno */
	int i; /* Indice */
	/* ----------------------------------------------------------------------------- */

	/* Controllo comando*/
	if(argc != 2) {
		fprintf(stderr, "Usage: /server <porta>\n");
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

	/* Aggiungo lo stdin al set principale */
	FD_SET(STDIN_FILENO, &master);

	/* Aggiungo il listener al set principale */
	FD_SET(listener, &master);

	/* Aggiorno il valore di fdmax */
	fdmax = listener;

	addrlen = sizeof(cl_addr);

	/* --- Init -------------------------------------------------------------------- */

	/* Inizializzo il menù */
	initMenu();

	/* Inizializzo i tavoli */
	initTavoli();

	printf("Server in ascolto sulla porta %d\n", atoi(argv[1]));

	/* --- Ciclo principale -------------------------------------------------------- */
	while(1) {

		/* Copio il set master in read_fds */
		read_fds = master;

		/* Attendo un evento */
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
					sock = malloc(sizeof(int));
					*sock = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
					if (*sock < 0){
						perror("Errore in fase di accept: \n");
						exit(-1);
					}
					printf("Nuova connessione da %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

					/* Creazione thread per la gestione della connessione */
					ret = pthread_create(&thread, NULL, socketHandler, (void*)sock);
				}else if(i == STDIN_FILENO) {
					/* Gestione input da tastiera */
					nbytes = read(i, command, sizeof(command));
					if (nbytes > 0) printf("%.*s", ret, command);
					else break;
				}
			}
		}

		
	}
	/* ----------------------------------------------------------------------------- */

	return 0;	
}

void* socketHandler(void* arg) {
	int sd = *((int*)arg); /* Socket per la connessione */
	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen;
	int nbytes; /* Numero di byte letti */
	cmd command; /* Comando da processare */

	addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	while(1){
		/* Gestione dati in arrivo da un client */
		nbytes = recv(sd, command, sizeof(command), 0);
		if (nbytes < 0){
			perror("Errore in fase di recv: \n");
			exit(-1);
		} else if(nbytes == 0) {
			/* Connessione chiusa */
			printf("Connessione chiusa da %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
			close(sd);
			break;
		} else if(nbytes > 0) {
			/* Dati ricevuti */
			printf("Comando %.*s ricevuto da %s:%d\n", (int)sizeof(command), command, inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

			/* Processamento comando */
			if(strcmp(command, PING) == 0) {
				ping(sd);
			}else if(strcmp(command, TD_MENU) == 0) {
				td_menu(sd);
			}else if(strcmp(command, CL_FIND) == 0) {
				cl_find(sd);
			}else if(strcmp(command, CL_BOOK) == 0) {
				cl_book(sd);
			}
		}
	}

	free(arg);
	pthread_exit(NULL);
}

