#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "header.h"

/* Mostra i dettagli dei comandi */
void help(void);

/* Mostra il menu */
void menu(int sd);

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
		fprintf(stderr, "Usage: /td <porta>\n");
		exit(-1);
	}
	/* --- Setup ------------------------------------------------------------------- */
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

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

	/* Aggiungo lo stdin al set principale */
	FD_SET(STDIN_FILENO, &master);

	/* Aggiungo il socket al set principale */
	FD_SET(sd, &master);

	/* --- Richesta identificativo del tavolo -------------------------------------- */

	/* --- Ciclo principale -------------------------------------------------------- */
	while(1){
		cmd command; /* Comando selezionato */


		printf("\033[H\033[J"); /* Pulizia schermo */
		printf("***************************** BENVENUTO *****************************\n");
		printf("Digita un comando: \033[s\n"); 
		printf("\n");
		printf("> help		--> mostra i dettagli dei comandi\n");
		printf("> menu		--> mostra il menu dei piatti\n");
		printf("> comanda	--> invia una comanda\n");
		printf("> conto		--> chiede il conto\n");
		printf("\033[u");
		fflush(stdout);

		/* Copio il set master in read_fds */
		read_fds = master;

		/* Attendo un evento */
		ret = select(sd+1, &read_fds, NULL, NULL, NULL);
		if (ret < 0){
			perror("Errore in fase di select: ");
			exit(-1);
		}

		/* Controllo se è arrivato qualcosa sullo stdin */
		if(FD_ISSET(STDIN_FILENO, &read_fds)){

			ret = scanf("%s", command);

			if(ret > 0){
				if(strcmp(command, "help") == 0){
					help();
				}
				else if(strcmp(command, "menu") == 0){
					menu(sd);
				}
				else if(strcmp(command, "comanda") == 0){
					/* comanda(sd); */
				}
				else if(strcmp(command, "conto") == 0){
					/* conto(sd); */
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
	printf("TableDevice scollegato.\n");

	return 0;
}

void help(void){
	printf("\033[H\033[J"); /* Pulizia schermo */
	printf("\033[1m\033[4m\033[34mmenu\033[0m\n");
	printf("Mostra il menu, cioè l'abbinamento fra codici, nomi dei piatti e relativi prezzi.\n");
	printf("\n");
	printf("\033[1m\033[4m\033[34mcomanda\033[0m \033[34m {<piatto_1-quantità_1>...<piatto_n-quantità_n>}\033[0m\n");
	printf("Invia una comanda alla cucina.\n");
	printf("\n");
	printf("\033[1m\033[4m\033[34mconto\033[0m\n");
	printf("Invia al server la richiesta di conto. Il server calcola il conto e lo invia al table device, che lo mostra a video.\n");
	printf("\nPremi INVIO per continuare...");
	getchar();
}

void menu(int sd){
	int ret; /* Valore di ritorno */
	unsigned int i; /* Indice */
	len n; /* numero piatti del menu */

	/* Richiesta menù */
	ret = send(sd, TD_MENU, sizeof(TD_MENU), 0); /* Invio richiesta del menu */
	if(ret < 0){
		perror("Errore in fase di richiesta del menù: \n");
		exit(-1);
	}

	ret = recv(sd, &n, sizeof(n), 0);
	if(ret < 0){
		perror("Errore in fase di ricezione del menù: \n");
		exit(-1);
	}
	n = ntohl(n);

	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Ricezione menu e stampa */
	for(i=0; i<n; i++){
		struct piatto p;
		ret = recv(sd, &p.tipo, sizeof(p.tipo), 0);
		if(ret < 0){
			perror("Errore in fase di ricezione del menù: \n");
			exit(-1);
		}

		ret = recv(sd, &p.nome, sizeof(p.nome), 0);
		if(ret < 0){
			perror("Errore in fase di ricezione del menù: \n");
			exit(-1);
		}

		ret = recv(sd, &p.prezzo, sizeof(p.prezzo), 0);
		if(ret < 0){
			perror("Errore in fase di ricezione del menù: \n");
			exit(-1);
		}
		p.prezzo = ntohl(p.prezzo);

		printf("%s - %s %d\n", p.tipo, p.nome, p.prezzo);
	}
	
	printf("\nPremi INVIO per continuare...");
	getchar();
}