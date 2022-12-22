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
		fprintf(stderr, "Usage: /kd <porta>\n");
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
		perror("Errore in fase di connessione");
		exit(-1);
	}

	/* Aggiungo lo stdin al set principale */
	FD_SET(STDIN_FILENO, &master);

	/* Aggiungo il socket al set principale */
	FD_SET(sd, &master);

	/* --- Ciclo principale -------------------------------------------------------- */
	while(1){
		
		printf("\033[H\033[J"); /* Pulizia schermo */


		printf("***************************** Kitchen Device *****************************\n");
		printf("Digita un comando: \033[s\n"); 
		printf("\n");
		printf("> take		--> accetta una comanda\n");
		printf("> show		--> mostra le comande accettate (in preparazione)\n");
		printf("> set		--> imposta lo stato della comanda\n");

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
			getchar();
			if(ret > 0){
				if(strcmp(command, "take") == 0){
					if(take(sd)) break;
				}
				else if(strcmp(command, "show") == 0){
					if(show(sd)) break;
				}
				else if(strcmp(command, "set") == 0){
					set(sd);
				}
				
			}

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
	printf("Kitchen Device scollegato.\n");

	return 0;
}