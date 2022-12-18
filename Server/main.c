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

/* --- Variabili Globali --------------------------------------------------------------- */

#define N_PIATTI 8 /* Numero massimo di piatti */
#define N_TAVOLI 10 /* Numero massimo di tavoli */

struct piatto menu[N_PIATTI]; /* Menu */
struct tavolo tavoli[N_TAVOLI]; /* Tavoli */

/* ------------------------------------------------------------------------------------- */


/* Gestore Socket */
void* socketHandler(void* arg);

/* --- Gestione Comandi -------------------- */

/* Risponde al ping */
void ping(int sd);

/* Invia il menù al TableDevice */
void td_menu(int sd);

/* Invia le disponibilità dei tavoli al Client */
void cl_find(int sd);

/* --- Funzioni di supporto ------------------- */

/* Inizializza il menù */
void initMenu(void);

/* Inizializza i tavoli */
void initTavoli(void);

/* Aggiunge una prenotazione al tavolo ordinata per data e ora */
void addPrenotazione(struct tavolo*, struct prenotazione*);

/* Cerca se è disponibile un posto per un tavolo per una prenotazione */
int findSlot(struct tavolo*, struct prenotazione*);

/* ----------------------------------------- */

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
			printf("Comando: %.*s ricevuto da %s:%d\n", (int)sizeof(command), command, inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

			/* Processamento comando */
			if(strcmp(command, PING) == 0) {
				ping(sd);
			}else if(strcmp(command, TD_MENU) == 0) {
				td_menu(sd);
			}else if(strcmp(command, CL_FIND) == 0) {
				cl_find(sd);
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
	len nlen; /* Variabile temporanea per il trasferimento*/
	unsigned int i; /* Indice */

	/* Invio il numero di elementi */
	nlen = htonl(N_PIATTI);
	send(sd, (void*)&nlen, sizeof(nlen), 0);
	
	/* Invio del menu */
	for(i=0; i<N_PIATTI; i++){
		uint32_t tmp = htonl(menu[i].prezzo);
		send(sd, (void*)&menu[i].tipo, sizeof(menu[i].tipo), 0);
		send(sd, (void*)&menu[i].nome, sizeof(menu[i].nome), 0);
		send(sd, (void*)&tmp, sizeof(tmp), 0);
	}
}

void cl_find(int sd){
	struct prenotazione p; /* Prenotazione da ricevere */
	struct tavolo* t[N_TAVOLI]; /* Tavoli liberi trovati */
	len nlen; /* numero di tavoli liberi trovati */
	len tmp; /* Variabile temporanea per il trasferimento*/
	int i; /* Indice */

	/* Ricezione prenotazione */
	recv(sd, p.cognome, sizeof(p.cognome), 0);
	recv(sd, &p.n_persone, sizeof(p.n_persone), 0);
	p.n_persone = ntohs(p.n_persone);
	recv(sd, &p.data.giorno, sizeof(p.data.giorno), 0);
	p.data.giorno = ntohs(p.data.giorno);
	recv(sd, &p.data.mese, sizeof(p.data.mese), 0);
	p.data.mese = ntohs(p.data.mese);
	recv(sd, &p.data.anno, sizeof(p.data.anno), 0);
	p.data.anno = ntohs(p.data.anno);
	recv(sd, &p.ora, sizeof(p.ora), 0);
	p.ora = ntohs(p.ora);

	/* Ricerca tavoli liberi */
	nlen = 0;
	for(i=0; i<N_TAVOLI; i++){
		if(findSlot(&tavoli[i], &p)){
			t[nlen++] = &tavoli[i];
		}
	}

	/* Invio numero di tavoli liberi trovati */
	tmp = htons(nlen);
	send(sd, &tmp, sizeof(tmp), 0);
	for(i=0; i<(int)nlen; i++){
		/* Invio tavoli liberi trovati */
		send(sd, &t[i]->id, sizeof(t[i]->id), 0);
		send(sd, &t[i]->sala, sizeof(t[i]->sala), 0);
		send(sd, t[i]->ubicazione, sizeof(t[i]->ubicazione), 0);
	}
}


/* --- FUNZIONI DI SUPPORTO --------------------------------------------------------------------------------------- */

void initMenu(void){
	memset(menu, 0, sizeof(menu)); /* Pulizia della struttura */

	/* Antipasti */
	strcpy(menu[0].tipo, "A1");
	strcpy(menu[0].nome, "Antipasto di terra");
	menu[0].prezzo = 7;

	strcpy(menu[1].tipo, "A2");
	strcpy(menu[1].nome, "Antipasto di mare");
	menu[1].prezzo = 8;

	/* Primi */
	strcpy(menu[2].tipo, "P1");
	strcpy(menu[2].nome, "Spaghetti alle vongole");
	menu[2].prezzo = 10;

	strcpy(menu[3].tipo, "P2");
	strcpy(menu[3].nome, "Rigatoni all'amatriciana");
	menu[3].prezzo = 6;

	/* Secondi */
	strcpy(menu[4].tipo, "S1");
	strcpy(menu[4].nome, "Frittura di calamari");
	menu[4].prezzo = 20;

	strcpy(menu[5].tipo, "S2");
	strcpy(menu[5].nome, "Arrosto misto");
	menu[5].prezzo = 15;

	/* Dolci */
	strcpy(menu[6].tipo, "D1");
	strcpy(menu[6].nome, "Crostata di mele");
	menu[6].prezzo = 5;

	strcpy(menu[7].tipo, "D2");
	strcpy(menu[7].nome, "Zuppa inglese");
	menu[7].prezzo = 5;
}

void initTavoli(void){
	memset(tavoli, 0, sizeof(tavoli)); /* Pulizia della struttura */

	tavoli[0].id = 1;
	tavoli[0].n_posti = 4;
	tavoli[0].sala = 1;
	strcpy(tavoli[0].ubicazione, "FINESTRA");
	pthread_mutex_init(&tavoli[0].mutex, NULL);

	tavoli[1].id = 2;
	tavoli[1].n_posti = 4;
	tavoli[1].sala = 1;
	strcpy(tavoli[1].ubicazione, "CAMINO");
	pthread_mutex_init(&tavoli[1].mutex, NULL);

	tavoli[2].id = 3;
	tavoli[2].n_posti = 6;
	tavoli[2].sala = 2;
	strcpy(tavoli[2].ubicazione, "INGRESSO");
	pthread_mutex_init(&tavoli[2].mutex, NULL);

	tavoli[3].id = 4;
	tavoli[3].n_posti = 6;
	tavoli[3].sala = 2;
	strcpy(tavoli[3].ubicazione, "CAMINO");
	pthread_mutex_init(&tavoli[3].mutex, NULL);

	tavoli[4].id = 5;
	tavoli[5].n_posti = 8;
	tavoli[4].sala = 3;
	strcpy(tavoli[4].ubicazione, "FINESTRA");
	pthread_mutex_init(&tavoli[4].mutex, NULL);

	tavoli[5].id = 6;
	tavoli[5].n_posti = 8;
	tavoli[5].sala = 3;
	strcpy(tavoli[5].ubicazione, "CAMINO");
	pthread_mutex_init(&tavoli[5].mutex, NULL);

	tavoli[6].id = 7;
	tavoli[6].n_posti = 10;
	tavoli[6].sala = 4;
	strcpy(tavoli[6].ubicazione, "INGRESSO");
	pthread_mutex_init(&tavoli[6].mutex, NULL);

	tavoli[7].id = 8;
	tavoli[7].n_posti = 10;
	tavoli[7].sala = 4;
	strcpy(tavoli[7].ubicazione, "CAMINO");
	pthread_mutex_init(&tavoli[7].mutex, NULL);

	tavoli[8].id = 9;
	tavoli[8].n_posti = 12;
	tavoli[8].sala = 5;
	strcpy(tavoli[8].ubicazione, "FINESTRA");
	pthread_mutex_init(&tavoli[8].mutex, NULL);

	tavoli[9].id = 10;
	tavoli[9].n_posti = 12;
	tavoli[9].sala = 5;
	strcpy(tavoli[9].ubicazione, "CAMINO");
	pthread_mutex_init(&tavoli[9].mutex, NULL);
}

void addPrenotazione(struct tavolo* t, struct prenotazione* pre){
	struct pre_list* tmp;
	struct pre_list* next;
	struct pre_list* new = (struct pre_list*)malloc(sizeof(struct pre_list));
	new->prenotazione = pre;
	new->next = NULL;

	pthread_mutex_lock(&t->mutex);

	if(t->prenotazioni == NULL){
		t->prenotazioni = new;
		pthread_mutex_unlock(&t->mutex);
		return;
	}

	/* Inserimento ordinato */
	for(tmp=next=t->prenotazioni; next != NULL; tmp=next,next=tmp->next){
		if(next->prenotazione->data.anno > pre->data.anno){
			new->next = next;
			tmp->next = new;
			break;
		}
		else if(next->prenotazione->data.anno == pre->data.anno){
			if(next->prenotazione->data.mese > pre->data.mese){
				new->next = next;
				tmp->next = new;
				break;
			}
			else if(next->prenotazione->data.mese == pre->data.mese){
				if(next->prenotazione->data.giorno > pre->data.giorno){
					new->next = next;
					tmp->next = new;
					break;
				}
				else if(next->prenotazione->data.giorno == pre->data.giorno){
					if(next->prenotazione->ora > pre->ora){
						new->next = next;
						tmp->next = new;
						break;
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&t->mutex);	
}

int findSlot(struct tavolo* t, struct prenotazione* pre){
	struct pre_list* tmp;

	if(t->n_posti < pre->n_persone)
		return 0;
	
	pthread_mutex_unlock(&t->mutex);

	for(tmp=t->prenotazioni; tmp != NULL; tmp=tmp->next){
		if(
			tmp->prenotazione->data.anno == pre->data.anno &&
			tmp->prenotazione->data.mese == pre->data.mese &&
			tmp->prenotazione->data.giorno == pre->data.giorno &&
			tmp->prenotazione->ora == pre->ora
		){
			pthread_mutex_unlock(&t->mutex);
			return 0;
		}
	}

	pthread_mutex_unlock(&t->mutex);
	return 1;
}
