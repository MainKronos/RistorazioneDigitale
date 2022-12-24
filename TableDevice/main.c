#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "main.h"


/* --- Main ------------------------------------------------------------------------ */

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	fd_set master; /* Set principale */
	fd_set read_fds; /* Set di lettura gestito dalla select */
	struct sockaddr_in sv_addr; /* Indirizzo server */
	int sd; /* Descrittore Socket */
	int sd_notify; /* Descrittore Socket per la notifica */
	int ret; /* Valore di ritorno */
	int lock; /* Blocco del tavolo */
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

	/* Creazione socket per la notifica */
	sd_notify = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo del server */
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);
	sv_addr.sin_port = htons(atoi(argv[1]));

	/* Connessione al server */
	if(
		(ret = connect(sd, (struct sockaddr *)&sv_addr, sizeof(sv_addr))) < 0 ||
		(ret = connect(sd_notify, (struct sockaddr *)&sv_addr, sizeof(sv_addr))) < 0
	){
		perror("Errore in fase di connessione");
		exit(-1);
	}

	/* Aggiungo lo stdin al set principale */
	FD_SET(STDIN_FILENO, &master);

	/* Aggiungo il socket al set principale */
	FD_SET(sd, &master);

	/* Aggiungo il socket per la notifica al set principale */
	FD_SET(sd_notify, &master);

	NC = 0; /* Numero di comande inviate */

	/* --- Richesta identificativo del tavolo -------------------------------------- */

	if(!connecttable(sd_notify)){ 
		lock = 0; /* !ALERT: Blocco il tavolo, mettere 0 in fase di test, altrimenti 1 */

	/* --- Ciclo principale -------------------------------------------------------- */
		while(1){
			
			printf("\033[H\033[J"); /* Pulizia schermo */

			if(lock){ /* Se il tavolo è bloccato */
				printf("***************************** TAVOLO %d *****************************\n", TID);
				printf("Inserisci codice di sblocco: \033[s\n");
			}else{
				printf("***************************** BENVENUTO *****************************\n");
				printNotifica();
				printf("Digita un comando: \033[s\n"); 
				printf("\n");
				printf("> help		--> mostra i dettagli dei comandi\n");
				printf("> menu		--> mostra il menu dei piatti\n");
				printf("> comanda	--> invia una comanda\n");
				printf("> conto		--> chiede il conto\n");
			}
			printf("\033[u");
			fflush(stdout);

			/* Copio il set master in read_fds */
			read_fds = master;

			/* Attendo un evento */
			ret = select(sd_notify+1, &read_fds, NULL, NULL, NULL);
			if (ret < 0){
				perror("Errore in fase di select");
				exit(-1);
			}

			/* Controllo se è arrivato qualcosa sullo stdin */
			if(FD_ISSET(STDIN_FILENO, &read_fds)){

				if(lock){
					unlock_code code; /* Codice di sblocco */
					scanf("%x", &code);
					getchar();
					if(unlocktable(sd, &lock, code)) break;
				}else{
					cmd command; /* Comando selezionato */

					ret = scanf("%s", command);
					getchar();
					if(ret > 0){
						if(strcmp(command, "help") == 0){
							if(help()) break;
						}
						else if(strcmp(command, "menu") == 0){
							if(menu(sd)) break;
						}
						else if(strcmp(command, "comanda") == 0){
							comanda(sd);
						}
						else if(strcmp(command, "conto") == 0){
							/* conto(sd); */
						}
					}
				}

				fflush(stdin);
			} else if(FD_ISSET(sd_notify, &read_fds)){
				if(!lock){ /* Se il tavolo non è bloccato */

					cmd command; /* Comando ricevuto */

					/* Leggo il comando */
					if((ret = read(sd_notify, command, sizeof(cmd))) <= 0){
						if(ret < 0) perror("Errore in fase di lettura");
						break;
					}

					if(strcmp(command, SV_UPTCOM) == 0){
						if(uptcom(sd_notify)) break;
					}else{
						/* Se è arrivato qualcosa di diverso dal socket sicuramente è un errore o è la chiusura del socket, quindi chiudo la connessione */
						break;
					}	
				}
			}else{
				/* Se è arrivato qualcosa di diverso dal socket sicuramente è un errore o è la chiusura del socket, quindi chiudo la connessione */
				break;
			}
		}
	}
	/* ----------------------------------------------------------------------------- */

	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Chiusura collegamento */
	close(sd);
	printf("TableDevice scollegato.\n");

	return 0;
}
