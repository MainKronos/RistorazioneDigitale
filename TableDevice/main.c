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

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	struct sockaddr_in sv_addr; /* Indirizzo server */
	int command; /* Comando selezionato */
	int sd; /* Socket */
	int ret; /* Valore di ritorno */
	/* ----------------------------------------------------------------------------- */

	/* Controllo comando*/
	if(argc != 2) {
		fprintf(stderr, "Usage: /td <porta>\n");
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
		printf("\033[H\033[J"); /* Pulizia schermo */
		printf("***************************** BENVENUTO *****************************\n");
		printf("Digita un comando: \033[s\n"); 
		printf("\n");
		printf("1) help			--> mostra i dettagli dei comandi\n");
		printf("2) menu			--> mostra il menu dei piatti\n");
		printf("3) comanda		--> invia una comanda\n");
		printf("4) conto		--> chiede il conto\n");
		printf("\033[u");
		ret = scanf("%d%*c", &command);

		if(ret > 0){
			switch (command){
				case 1:
					help();
					break;
				case 2:
					/* menu(sd); */
					break;
				
				default:
					break;
			}
		}

	}
	/* ----------------------------------------------------------------------------- */

	/* Chiusura collegamento */
	close(sd);
}

void help(void){
	printf("\033[H\033[J"); /* Pulizia schermo */
	printf("\033[1m\033[4m\033[34mmenu\033[0m\n");
	printf("Mostra il menu, cioè l'abbinamento fra codici, nomi dei piatti e relativi prezzi.\n");
	printf("\n");
	printf("\033[1m\033[4m\033[34mcomanda\033[0m \033[34m {<piatto_1-quantità_1>...<piatto_n-quantità_n>}\033[0m\n");
	printf("invia una comanda alla cucina.\n");
	printf("\n");
	printf("\033[1m\033[4m\033[34mconto\033[0m\n");
	printf("Invia al server la richiesta di conto. Il server calcola il conto e lo invia al table device, che lo mostra a video.\n");
	getchar();
}