#include "header.h"

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
	tavoli[4].n_posti = 8;
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

int bookSlot(struct tavolo* t, struct prenotazione* p){
	struct pre_list* pre;
	struct pre_list* succ;
	struct pre_list* new = (struct pre_list*)malloc(sizeof(struct pre_list));
	new->prenotazione = p;
	new->next = NULL;

	pthread_mutex_lock(&t->mutex);

	if(t->prenotazioni == NULL){
		t->prenotazioni = new;
		pthread_mutex_unlock(&t->mutex);
		return 1;
	}

	/* Inserimento ordinato */
	for(pre=succ=t->prenotazioni; succ != NULL; pre=succ,succ=succ->next){
		if(difftime(succ->prenotazione->datetime, p->datetime) == 0){
			pthread_mutex_unlock(&t->mutex);
			return 0;
		}

		if(difftime(succ->prenotazione->datetime, p->datetime) > 0){
			new->next = succ;
			pre->next = new;
			pthread_mutex_unlock(&t->mutex);
			return 1;
		}
	}

	pthread_mutex_unlock(&t->mutex);	
	return 0;
}

int findSlot(struct tavolo* t, struct prenotazione pre){
	struct pre_list* tmp;

	if(t->n_posti < pre.n_persone)
		return 0;
	
	pthread_mutex_lock(&t->mutex);

	for(tmp=t->prenotazioni; tmp != NULL; tmp=tmp->next){
		if(difftime(tmp->prenotazione->datetime, pre.datetime)==0){
			pthread_mutex_unlock(&t->mutex);
			return 0;
		}
	}

	pthread_mutex_unlock(&t->mutex);
	return 1;
}

struct pre_sosp* findPrenotazioneSospesa(int sd){
	
	struct pre_sosp* p;

	pthread_mutex_lock(&mutex_prenotazioni_sospese);

	for(p = prenotazioni_sospese; p != NULL; p = p->next){
		if(p->sd == sd){
			break;
		}
	}
	pthread_mutex_unlock(&mutex_prenotazioni_sospese);
	return p;
}

void removePrenotazioneSospesa(int sd){
	
	struct pre_sosp* p_sosp; /* Variabile temporanea per scorrere la lista di prenotazioni sospese */
	struct pre_sosp* p_sosp_prev = NULL; /* Variabile temporanea che si riferisce all'elemento precedente di p_sosp */

	pthread_mutex_lock(&mutex_prenotazioni_sospese);

	for(p_sosp = prenotazioni_sospese; p_sosp != NULL; p_sosp_prev = p_sosp, p_sosp = p_sosp->next){
		if(p_sosp->sd == sd){ /* Se l'elemento corrente è quello da eliminare */
			if(p_sosp_prev == NULL){
				prenotazioni_sospese = p_sosp->next;
			}else{
				p_sosp_prev->next = p_sosp->next;
			}
			free(p_sosp); /* Deallocazione della memoria occupata dalla prenotazione sospesa */
			break;
		}
	}
	pthread_mutex_unlock(&mutex_prenotazioni_sospese);
}

void insertPrenotazioneSospesa(struct pre_sosp* p_sosp){
	struct pre_sosp* tmp; /* Variabile temporanea per scorrere la lista di prenotazioni sospese */

	pthread_mutex_lock(&mutex_prenotazioni_sospese);
	
	if(prenotazioni_sospese == NULL){
		prenotazioni_sospese = p_sosp;
	}else{
		for(tmp = prenotazioni_sospese; tmp->next != NULL; tmp = tmp->next);
		tmp->next = p_sosp;
	}

	pthread_mutex_unlock(&mutex_prenotazioni_sospese);
}