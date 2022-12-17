#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "header.h"

/* Gestore Socket */
void* socketHandler(void* arg);

/* --- Gestione Comandi ----------- */
/* Risponde al ping */
void ping(int sd);

/* Invia il menù al TableDevice */
void td_menu(int sd);


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

	/* ----------------------------------------------------------------------------- */

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
			printf("Dati ricevuti da %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
			printf("Comando: %.*s\n", (int)sizeof(command), command);

			/* Processamento comando */
			if(strcmp(command, PING) == 0) {
				ping(SO_DETACH_REUSEPORT_BPF);
			}else if(strcmp(command, TD_MENU) == 0) {
				td_menu(sd);
			}
		}
	}

	free(arg);
	pthread_exit(NULL);
}

/* --- COMANDI ---------------------------------------------------------------------------------------------------- */

void ping(int sd){
	char buffer[] = "pong";
	send(sd, (void*)buffer, sizeof(buffer), 0);
}

void td_menu(int sd){
	static int loaded = 0; /* Se il menu è già stato caricato */
	static struct piatto p[M_LEN]; /* Menu */
	int i; /* Indice */

	/* Controllo se il menu è stato inizzializzato, altrimenti lo inizializzo */
	if(!loaded){	
		/* Inizializzazione menu */

		/* Antipasti */
		strcpy(p[0].tipo, "A1");
		strcpy(p[0].nome, "Antipasto di terra");
		p[0].prezzo = 7;

		strcpy(p[1].tipo, "A2");
		strcpy(p[1].nome, "Antipasto di mare");
		p[1].prezzo = 8;

		/* Primi */
		strcpy(p[2].tipo, "P1");
		strcpy(p[2].nome, "Spaghetti alle vongole");
		p[2].prezzo = 10;

		strcpy(p[3].tipo, "P2");
		strcpy(p[3].nome, "Rigatoni all'amatriciana");
		p[3].prezzo = 6;

		/* Secondi */
		strcpy(p[4].tipo, "S1");
		strcpy(p[4].nome, "Frittura di calamari");
		p[4].prezzo = 20;

		strcpy(p[5].tipo, "S2");
		strcpy(p[5].nome, "Arrosto misto");
		p[5].prezzo = 15;

		/* Dolci */
		strcpy(p[6].tipo, "D1");
		strcpy(p[6].nome, "Crostata di mele");
		p[6].prezzo = 5;

		strcpy(p[7].tipo, "D2");
		strcpy(p[7].nome, "Zuppa inglese");
		p[7].prezzo = 5;
		
		loaded = 1;
	}
	
	/* Invio del menu */
	for(i=0; i<M_LEN; i++){
		u_int32_t tmp = htonl(p[i].prezzo);
		send(sd, (void*)&p[i].tipo, sizeof(p[i].tipo), 0);
		send(sd, (void*)&p[i].nome, sizeof(p[i].nome), 0);
		send(sd, (void*)&tmp, sizeof(tmp), 0);
	}
}
