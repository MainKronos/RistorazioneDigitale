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

/* --- Strutture ------------------------------------------------------------------- */

/* Lista di piatti della comanda usato per il parsing da input */
struct p_com{
	type code; /* codice del piatto */
	len num; /* quantità del piatto */
	struct p_com* next; /* puntatore al prossimo piatto */
};

tavolo_id TID; /* Identificativo del tavolo */

/* --- Funzioni ------------------------------------------------------------------- */

/* Connette il dispositivo ad un tavolo */
int connectTable(int);

/* Sblocca il dispositivo */
int unlockTable(int, int*, unlock_code);

/* Mostra i dettagli dei comandi */
int help(void);

/* Mostra il menu */
int menu(int);

/* Invia una comanda al server */
int comanda(int);

/* --- Utility --------------------------------------------------------------------- */

/* Aggiunge un piatto alla lista comanda */
int addPiattoToComanda(struct p_com**, type, len);

/* Converte la lista comanda in una comanda */
int p_comToComanda(struct p_com*, struct comanda*);

/* --- Main ------------------------------------------------------------------------ */

int main(int argc, char *argv[]){

	/* --- Variabili --------------------------------------------------------------- */
	fd_set master; /* Set principale */
	fd_set read_fds; /* Set di lettura gestito dalla select */
	struct sockaddr_in sv_addr; /* Indirizzo server */
	int sd; /* Descrittore Socket */
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

	/* --- Richesta identificativo del tavolo -------------------------------------- */

	if(!connectTable(sd)){ 
		lock = 0; /* Blocco il tavolo, mettere 0 in fase di test, altrimenti 1 */

	/* --- Ciclo principale -------------------------------------------------------- */
		while(1){
			
			printf("\033[H\033[J"); /* Pulizia schermo */

			if(lock){ /* Se il tavolo è bloccato */
				printf("***************************** TAVOLO %d *****************************\n", TID);
				printf("Inserisci codice di sblocco: \033[s\n");
			}else{
				printf("***************************** BENVENUTO *****************************\n");
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
			ret = select(sd+1, &read_fds, NULL, NULL, NULL);
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
					if(unlockTable(sd, &lock, code)) break;
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
			} else if(FD_ISSET(sd, &read_fds)){
				/* Se è arrivato qualcosa dal socket sicuramente è un errore o è la chiusura del socket, quindi chiudo la connessione */
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

int connectTable(int sd){
	int ret; /* Valore di ritorno */
	tavolo_id tmp; /* Variabile temporanea */

	/* Invio richiesta id del tavolo */
	if(send(sd, TD_GETID, sizeof(TD_GETID), 0) < 0) { 
		perror("Errore in fase di richiesta dell'id del tavolo");
		return -1;
	}

	/* Ricezione id del tavolo */
	if((ret = recv(sd, &tmp, sizeof(tmp), 0)) <= 0){
		if(ret<0) perror("Errore in fase di ricezione dell'id del tavolo");
		return -1;
	}

	tmp = ntohl(tmp);

	/* Se l'ID del tavolo è -1 significa che non ci sono più tavoli liberi da associare */
	if(tmp == (tavolo_id)-1){
		return -1;
	}

	TID = tmp;

	return 0;
}

int unlockTable(int sd, int* lock, unlock_code code){
	int ret; /* Valore di ritorno */
	uint8_t r_lock; /* Variabile per la risposta di unlock */
	response res; /* Variabile per la risposta del server */

	printf("\033[H\033[J"); /* Pulizia schermo */
	
	/* Invio richiesta di sblocco */
	if(send(sd, TD_UNLOCK, sizeof(TD_UNLOCK), 0) < 0){ 
		perror("Errore in fase di richiesta di sblocco");
		return -1;
	}


	/* Invio codice di sblocco */
	code = htonl(code);
	if(send(sd, &code, sizeof(unlock_code), 0) < 0){ 
		perror("Errore in fase di invio del codice di sblocco");
		return -1;
	}

	/* Ricezione risposta di unlock */
	if((ret = recv(sd, &r_lock, sizeof(uint8_t), 0)) <= 0){ 
		if(ret<0) perror("Errore in fase di ricezione della risposta di sblocco");
		return -1;
	}

	if(!r_lock) *lock = 0;

	/* Ricezione risposta del server */
	if((ret = recv(sd, &res, sizeof(response), 0)) <= 0){ 
		if(ret<0) perror("Errore in fase di ricezione della risposta del server");
		return -1;
	}

	printf("%s\n", res);
	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}

int help(void){
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

	return 0;
}

int menu(int sd){
	int ret; /* Valore di ritorno */
	unsigned int i; /* Indice */
	len n; /* numero piatti del menu */

	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Invio richiesta del menu */
	if(send(sd, TD_MENU, sizeof(TD_MENU), 0) < 0){ 
		perror("Errore in fase di richiesta del menù");
		return -1;
	}

	/* Ricezione numero piatti del menu */
	if(recv(sd, &n, sizeof(n), 0) < 0){
		perror("Errore in fase di ricezione del menù");
		return -1;
	}
	n = ntohl(n);

	/* Ricezione menu e stampa */
	for(i=0; i<n; i++){
		struct piatto p;

		if(
			(ret = recv(sd, &p.code, sizeof(p.code), 0)) < 0 ||
			(ret = recv(sd, &p.nome, sizeof(p.nome), 0)) < 0 ||
			(ret = recv(sd, &p.prezzo, sizeof(p.prezzo), 0)) < 0
		){
			if(ret<0) perror("Errore in fase di ricezione del menù");
			return -1;
		}
		p.prezzo = ntohl(p.prezzo);

		printf("%-2s - %-30s %d\n", p.code, p.nome, p.prezzo);
	}
	
	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}

int comanda(int sd){
	len n_piatti; /* numero piatti della comanda */
	struct p_com* com_ptr; /* puntatore alla lista di piatti della comanda */
	struct comanda com; /* struttura comanda per il traferimento */
	type code_tmp; /* variabile temporanea del codice del piatto */
	len num_tmp; /* variabile temporanea della quantità del piatto */
	int i; /* indice */
	char buffer[5]; /* buffer per parsare l'input dei piatti */
	char c; /* carattere temporaneo */
	int sw; /* variabile di controllo per il parser */
	response r; /* risposta del server */
	len tmp; /* variabile temporanea per l'invio */
	tavolo_id tmp_t; /* variabile temporanea per l'invio */

	printf("\033[H\033[J"); /* Pulizia schermo */

	/*************************************************************************
	Blocco di codice per parsare l'input dei piatti.
	NON MODIFICARE ASSOLUTAMENTE .
	**************************************************************************/

	n_piatti = 0;
	com_ptr = NULL;
	i=0;
	sw = 0;
	while(1){
		c = getchar();
		buffer[i] = c;
		if(c == '-' || c == ' ' || c == '\n'){
			buffer[i] = '\0';
			if(sw){
				/* Quantità piatto */
				num_tmp = atoi(buffer);
				addPiattoToComanda(&com_ptr, code_tmp, num_tmp);
				n_piatti++;
				sw = 0;
			}else{
				/* Codice piatto */
				if(sscanf(buffer, "%s", code_tmp) != -1) sw = 1;
			}
			i=0;
		}
		else i++;
		if(c == '\n') break;
	}

	/*****************************************************************************/

	/* Converto la lista comanda nel formato standard per il trasferimento */

	memset(&com, 0, sizeof(com)); /* Pulizia struttura */
	com.nlen = n_piatti;
	com.tid = TID;
	p_comToComanda(com_ptr, &com);

	com_ptr = NULL; /* Pulizia lista comanda */


	/* Invio richiesta di invio comanda */
	if(send(sd, TD_COMANDA, sizeof(TD_COMANDA), 0) < 0){
		perror("Errore in fase di invio richiesta di invio comanda");
		return -1;
	}

	/* Invio identificativo tavolo */
	tmp_t = htonl(com.tid);
	if(send(sd, &tmp_t, sizeof(tmp_t), 0) < 0){
		perror("Errore in fase di invio identificativo tavolo");
		return -1;
	}

	/* Invio numero piatti della comanda */
	tmp = htonl(com.nlen);
	if(send(sd, &tmp, sizeof(tmp), 0) < 0){
		perror("Errore in fase di invio numero piatti");
		return -1;
	}
	for(i=0; i<(int)com.nlen; i++){
		/* Invio codice piatto e quantità*/
		
		tmp = htonl(com.q[i]);
		if(
			send(sd, com.p[i], sizeof(type), 0) < 0 ||
			send(sd, &tmp, sizeof(len), 0) < 0
		){
			perror("Errore in fase di invio codice piatto");
			return -1;
		}
	}

	free(com.p); /* Libero memoria allocata per la lista di codici */
	free(com.q); /* Libero memoria allocata per la lista di quantità */

	/* Ricezione risposta dal server */
	if(recv(sd, r, sizeof(r), 0) < 0){
		perror("Errore in fase di ricezione risposta dal server");
		return -1;
	}

	printf("%s\n", r);
	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}


int addPiattoToComanda(struct p_com** com_ptr, type code, len num){
	struct p_com* tmp; /* puntatore temporaneo */
	struct p_com* pre_tmp; /* puntatore precedente di tmp */

	if(*com_ptr != NULL){
	
		for(tmp=*com_ptr; tmp!=NULL; pre_tmp=tmp, tmp=tmp->next){
			/* Se il codice è già presente nella lista, aggiorno la quantità */
			if(strcmp(tmp->code, code) == 0){
				tmp->num += num;
				return 0;
			}
		}

		/* Altrimenti aggiungo il nuovo elemento alla fine della lista */
		tmp = malloc(sizeof(struct p_com));
		pre_tmp->next = tmp;

	}else{
		tmp = malloc(sizeof(struct p_com));
		*com_ptr = tmp;
	}

	strcpy(tmp->code,code);
	tmp->num = num;
	tmp->next = NULL;

	return 0;
}

int p_comToComanda(struct p_com* com_ptr, struct comanda* comanda){
	int i; /* indice */
	struct p_com* tmp; /* puntatore temporaneo */

	comanda->p = malloc(sizeof(type)*comanda->nlen);
	comanda->q = malloc(sizeof(len)*comanda->nlen);

	for(i=0; i<(int)comanda->nlen; i++){
		strcpy(comanda->p[i], com_ptr->code);
		comanda->q[i] = com_ptr->num;
		tmp = com_ptr;
		com_ptr=com_ptr->next;
		free(tmp); /* Libero la memoria occupata dalla lista */
	}
	
	return 0;
}