#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "cardinal.c"

/* Mostra i dettagli dei comandi */
int help(void);

/* Ricerca la disponibilità per una prenotazione */
int find(int sd);

/* Invia una prenotazione */
int book(int sd);

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	fd_set master; /* Set principale */
	fd_set read_fds; /* Set di lettura gestito dalla select */
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

	UNUSED(argv);
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	
	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo del server */
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);
	sv_addr.sin_port = htons(4242);

	/* Connessione al server */
	ret = connect(sd, (struct sockaddr *)&sv_addr, sizeof(sv_addr));
	if (ret < 0){
		perror("Errore in fase di connessione: \n");
		exit(-1);
	}

	/* Aggiungo lo stdin al set principale */
	FD_SET(STDIN_FILENO, &master);

	/* Aggiungo il socket al set principale */
	FD_SET(sd, &master);

	/* --- Ciclo principale -------------------------------------------------------- */
	while(1){
		printf("\033[H\033[J"); /* Pulizia schermo */
		printf("***************************** CLIENT *****************************\n");
		printf("Digita un comando: \033[s\n"); 
		printf("\n");
		printf("> help		--> mostra i dettagli dei comandi\n");
		printf("> find		--> ricerca la disponibilità per una prenotazione\n");
		printf("> book		--> invia una prenotazione\n");
		printf("> esc		--> termina il client\n");
		printf("\033[u");
		fflush(stdout);

		/* Copio il set master in read_fds */
		read_fds = master;

		/* Attendo un evento */
		ret = select(sd+1, &read_fds, NULL, NULL, NULL);
		if (ret < 0){
			perror("Errore in fase di select");
			exit(-1);
		}

		/* Controllo se è arrivato qualcosa sullo stdin */
		if(FD_ISSET(STDIN_FILENO, &read_fds)){
			cmd command; /* Comando selezionato */

			ret = scanf("%s", command);

			if(ret > 0){
				if(strcmp(command, "help") == 0){
					if(help()) break;
				}
				else if(strcmp(command, "find") == 0){
					if(find(sd)) break;
				}
				else if(strcmp(command, "book") == 0){
					if(book(sd)) break;
				}
				else if(strcmp(command, "esc") == 0){
					break;
				}
			}
			scanf("%*c"); /* Pulizia buffer */
			fflush(stdin);
		} else if(FD_ISSET(sd, &read_fds)){
			/* Se è arrivato qualcosa dal socket sicuramente è un errore o è la chiusura del socket, quindi chiudo la connessione */
			break;
		}

	}
	/* ----------------------------------------------------------------------------- */

	printf("\033[H\033[J"); /* Pulizia schermo */
	/* Chiusura collegamento */
	close(sd);
	printf("Client scollegato.\n");

	return 0;
}

int help(void){
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
	return 0;
}

int find(int sd){
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
		send(sd, CL_FIND, sizeof(CL_FIND), 0); /* Invio richiesta del menu */

		/* Invio prenotazione */
		send(sd, p.cognome, sizeof(p.cognome), 0);
		n_persone = htons(p.n_persone);
		send(sd, &n_persone, sizeof(n_persone), 0);
		timestamp = htonl(p.datetime);
		send(sd, &timestamp, sizeof(timestamp), 0);

		/* Ricezione numero tavoli */
		ret = recv(sd, &n, sizeof(n), 0);
		if(ret<=0){
			if(ret<0) perror("find");
			return -1;
		}
		n = ntohs(n);

		if(n == 0){
			printf("Nessun tavolo disponibile\n");
		}
		else{
			printf("Tavoli disponibili:\n");
			for(i = 0; i < (int)n; i++){
				ret = recv(sd, &t.id, sizeof(t.id), 0);
				if(ret<=0){
					if(ret<0) perror("find");
					return -1;
				}
				ret = recv(sd, &t.sala, sizeof(t.sala), 0);
				if(ret<=0){
					if(ret<0) perror("find");
					return -1;
				}
				ret = recv(sd, t.ubicazione, sizeof(t.ubicazione), 0);
				if(ret<=0){
					if(ret<0) perror("find");
					return -1;
				}

				printf("%d) T%-5d SALA%-5d %s\n", i, t.id, t.sala, t.ubicazione);
			}
		}
	}else{
		printf("La data %.24s non è valida \n", ctime(&p.datetime));
	}

	printf("\nPremi INVIO per continuare...");
	getchar(); 
	return 0;
}

int book(int sd){
	int ret; /* Valore di ritorno */
	len choice; /* Scelta dell'utente */
	response r; /* Risposta dal server */

	scanf("%d", &choice);
	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Invio scelta */
	/* Richiesta menù */
	send(sd, CL_BOOK, sizeof(CL_BOOK), 0); /* Invio richiesta prenotazione tavolo */

	choice = htonl(choice);
	send(sd, &choice, sizeof(choice), 0); /* Invio scelta */

	/* Ricezione conferma */
	ret = recv(sd, r, sizeof(r), 0);
	if(ret<=0){
		if(ret<0) perror("book");
		return -1;
	}
	printf("%s\n", r);

	printf("\nPremi INVIO per continuare...");
	getchar(); 
	return 0;
}
