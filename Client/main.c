#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "header.h"

/* Mostra i dettagli dei comandi */
void help(void);

/* Ricerca la disponibilità per una prenotazione */
void find(int sd);

/* Invia una prenotazione */
void book(int sd);

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	struct sockaddr_in sv_addr; /* Indirizzo server */
	int sd; /* Descrittore Socket */
	int ret; /* Valore di ritorno */
	/* ----------------------------------------------------------------------------- */

	/* Controllo comando*/
	if(argc != 2) {
		fprintf(stderr, "Usage: /cli <porta>\n");
		exit(-1);
	}
	/* --- Setup ------------------------------------------------------------------- */
	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo del server */
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);
	sv_addr.sin_port = htons(atoi(argv[1]));

	/* Connessione al server */
	ret = connect(sd, (struct sockaddr *)&sv_addr, sizeof(sv_addr));
	if (ret < 0){
		perror("Errore in fase di connessione: \n");
		exit(-1);
	}
	/* ----------------------------------------------------------------------------- */

	/* --- Ciclo principale -------------------------------------------------------- */
	while(1){
		cmd command; /* Comando selezionato */

		printf("\033[H\033[J"); /* Pulizia schermo */
		printf("***************************** CLIENT *****************************\n");
		printf("Digita un comando: \033[s\n"); 
		printf("\n");
		printf("1) help		--> mostra i dettagli dei comandi\n");
		printf("2) find		--> ricerca la disponibilità per una prenotazione\n");
		printf("3) book		--> invia una prenotazione\n");
		printf("4) esc		--> termina il client\n");
		printf("\033[u");

		ret = scanf("%s", command);

		if(ret > 0){
			if(strcmp(command, "help") == 0){
				help();
			}
			else if(strcmp(command, "find") == 0){
				find(sd);
			}
			else if(strcmp(command, "book") == 0){
				book(sd);
			}
			else if(strcmp(command, "esc") == 0){
				printf("\033[H\033[J"); /* Pulizia schermo */
				break;
			}
		}
		scanf("%*c");
		fflush(stdin);

	}
	/* ----------------------------------------------------------------------------- */

	/* Chiusura collegamento */
	close(sd);

	return 0;
}

void help(void){
	printf("\033[H\033[J"); /* Pulizia schermo */
	printf("\033[1m\033[4m\033[34mfind\033[0m \033[34mcognome persone data ora\033[0m\n");
	printf("Invia una richiesta di disponibilità dove la data è espressa in formato GG-MM-AA e l'ora come HH.\n");
	printf("\n");
	printf("\033[1m\033[4m\033[34mbook\033[0m \033[34mopz\033[0m\n");
	printf("Invia una richiesta di prenotazione per l'opzione opz.\n");
	printf("\n");
	printf("\033[1m\033[4m\033[34mesc\033[0m\n");
	printf("Termina il client\n");
	printf("\nPremi INVIO per continuare...");
	getchar();
}

void find(int sd){
	int ret; /* Valore di ritorno */
	int i; /* Indice */
	struct tm datetime; /* Variabile temporanea per contenere la data e l'ora */
	time_t timestamp; /* Variabile temporanea l'invio del timestamp della prenotazione */
	uint16_t n_persone; /* Variabile temporanea per l'invio del numero di persone */
	struct prenotazione p; /* Prenotazione da inviare */
	len n; /* Numero di tavoli trovati */
	struct tavolo t; /* buffer per i tavolo trovati */

	memset(&datetime, 0, sizeof(datetime)); /* Pulizia struttura */
	memset(&p, 0, sizeof(p)); /* Pulizia struttura */
	memset(&t, 0, sizeof(t)); /* Pulizia struttura */

	scanf("%s %hu %d-%d-%d %d", p.cognome, &p.n_persone, &datetime.tm_mday, &datetime.tm_mon, &datetime.tm_year, &datetime.tm_hour);

	datetime.tm_mon -= 1; /* Correzione mese */
	datetime.tm_year += 100; /* Correzione anno */

	p.datetime = mktime(&datetime); /* Conversione in timestamp */

	printf("\033[H\033[J"); /* Pulizia schermo */

	if(difftime(p.datetime, time(NULL)) > 0){

		/* Richiesta menù */
		ret = send(sd, CL_FIND, sizeof(CL_FIND), 0); /* Invio richiesta del menu */
		if(ret < 0){
			perror("Errore in fase di richiesta del menù: \n");
			exit(-1);
		}

		/* Invio prenotazione */
		send(sd, p.cognome, sizeof(p.cognome), 0);
		n_persone = htons(p.n_persone);
		send(sd, &n_persone, sizeof(n_persone), 0);
		timestamp = htonl(p.datetime);
		send(sd, &timestamp, sizeof(timestamp), 0);

		/* Ricezione numero tavoli */
		recv(sd, &n, sizeof(n), 0);
		n = ntohs(n);

		if(n == 0){
			printf("Nessun tavolo disponibile\n");
		}
		else{
			printf("Tavoli disponibili:\n");
			for(i = 0; i < (int)n; i++){
				recv(sd, &t.id, sizeof(t.id), 0);
				recv(sd, &t.sala, sizeof(t.sala), 0);
				recv(sd, t.ubicazione, sizeof(t.ubicazione), 0);

				printf("%d) T%-5d SALA%-5d %s\n", i, t.id, t.sala, t.ubicazione);
			}
		}
	}else{
		printf("La data %.24s non è valida \n", ctime(&p.datetime));
	}

	printf("\nPremi INVIO per continuare...");
	getchar(); 
}

void book(int sd){
	int ret; /* Valore di ritorno */
	len choice; /* Scelta dell'utente */
	response r; /* Risposta dal server */

	scanf("%u", &choice);
	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Invio scelta */
	/* Richiesta menù */
	ret = send(sd, CL_BOOK, sizeof(CL_BOOK), 0); /* Invio richiesta prenotazione tavolo */
	if(ret < 0){
		perror("Errore in fase di invio della selezione tavolo: \n");
		exit(-1);
	}

	choice = htonl(choice);
	send(sd, &choice, sizeof(choice), 0); /* Invio scelta */

	/* Ricezione conferma */
	recv(sd, r, sizeof(r), 0);
	printf("%s\n", r);

	printf("\nPremi INVIO per continuare...");
	getchar(); 
}
