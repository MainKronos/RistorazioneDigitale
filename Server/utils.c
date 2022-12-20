#include "header.h"
#include "main.h"

/* --- FUNZIONI DI SUPPORTO --------------------------------------------------------------------------------------- */

void initMenu(void){
	memset(menu, 0, sizeof(menu)); /* Pulizia della struttura */

	/* Antipasti */
	strcpy(menu[0].code, "A1");
	strcpy(menu[0].nome, "Antipasto di terra");
	menu[0].prezzo = 7;

	strcpy(menu[1].code, "A2");
	strcpy(menu[1].nome, "Antipasto di mare");
	menu[1].prezzo = 8;

	/* Primi */
	strcpy(menu[2].code, "P1");
	strcpy(menu[2].nome, "Spaghetti alle vongole");
	menu[2].prezzo = 10;

	strcpy(menu[3].code, "P2");
	strcpy(menu[3].nome, "Rigatoni all'amatriciana");
	menu[3].prezzo = 6;

	/* Secondi */
	strcpy(menu[4].code, "S1");
	strcpy(menu[4].nome, "Frittura di calamari");
	menu[4].prezzo = 20;

	strcpy(menu[5].code, "S2");
	strcpy(menu[5].nome, "Arrosto misto");
	menu[5].prezzo = 15;

	/* Dolci */
	strcpy(menu[6].code, "D1");
	strcpy(menu[6].nome, "Crostata di mele");
	menu[6].prezzo = 5;

	strcpy(menu[7].code, "D2");
	strcpy(menu[7].nome, "Zuppa inglese");
	menu[7].prezzo = 5;
}

void initTavoli(void){
	memset(tavoli, 0, sizeof(tavoli)); /* Pulizia della struttura */

	tavoli[0].inf.id = 0;
	tavoli[0].inf.n_posti = 4;
	tavoli[0].inf.sala = 1;
	strcpy(tavoli[0].inf.ubicazione, "FINESTRA");
	tavoli[0].sd = -1;
	pthread_mutex_init(&tavoli[0].mutex, NULL);

	tavoli[1].inf.id = 1;
	tavoli[1].inf.n_posti = 4;
	tavoli[1].inf.sala = 1;
	strcpy(tavoli[1].inf.ubicazione, "CAMINO");
	tavoli[1].sd = -1;
	pthread_mutex_init(&tavoli[1].mutex, NULL);

	tavoli[2].inf.id = 2;
	tavoli[2].inf.n_posti = 6;
	tavoli[2].inf.sala = 2;
	strcpy(tavoli[2].inf.ubicazione, "INGRESSO");
	tavoli[2].sd = -1;
	pthread_mutex_init(&tavoli[2].mutex, NULL);

	tavoli[3].inf.id = 3;
	tavoli[3].inf.n_posti = 6;
	tavoli[3].inf.sala = 2;
	strcpy(tavoli[3].inf.ubicazione, "CAMINO");
	tavoli[3].sd = -1;
	pthread_mutex_init(&tavoli[3].mutex, NULL);

	tavoli[4].inf.id = 4;
	tavoli[4].inf.n_posti = 8;
	tavoli[4].inf.sala = 3;
	strcpy(tavoli[4].inf.ubicazione, "FINESTRA");
	tavoli[4].sd = -1;
	pthread_mutex_init(&tavoli[4].mutex, NULL);

	tavoli[5].inf.id = 5;
	tavoli[5].inf.n_posti = 8;
	tavoli[5].inf.sala = 3;
	strcpy(tavoli[5].inf.ubicazione, "CAMINO");
	tavoli[5].sd = -1;
	pthread_mutex_init(&tavoli[5].mutex, NULL);

	tavoli[6].inf.id = 6;
	tavoli[6].inf.n_posti = 10;
	tavoli[6].inf.sala = 4;
	strcpy(tavoli[6].inf.ubicazione, "INGRESSO");
	tavoli[6].sd = -1;
	pthread_mutex_init(&tavoli[6].mutex, NULL);

	tavoli[7].inf.id = 7;
	tavoli[7].inf.n_posti = 10;
	tavoli[7].inf.sala = 4;
	strcpy(tavoli[7].inf.ubicazione, "CAMINO");
	tavoli[7].sd = -1;
	pthread_mutex_init(&tavoli[7].mutex, NULL);

	tavoli[8].inf.id = 8;
	tavoli[8].inf.n_posti = 12;
	tavoli[8].inf.sala = 5;
	strcpy(tavoli[8].inf.ubicazione, "FINESTRA");
	tavoli[8].sd = -1;
	pthread_mutex_init(&tavoli[8].mutex, NULL);

	tavoli[9].inf.id = 9;
	tavoli[9].inf.n_posti = 12;
	tavoli[9].inf.sala = 5;
	strcpy(tavoli[9].inf.ubicazione, "CAMINO");
	tavoli[9].sd = -1;
	pthread_mutex_init(&tavoli[9].mutex, NULL);
}

int bookSlot(struct tavolo_sv* t, struct prenotazione_sv* p){
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
		if(difftime(succ->prenotazione->inf.datetime, p->inf.datetime) == 0){
			pthread_mutex_unlock(&t->mutex);
			return 0;
		}

		if(difftime(succ->prenotazione->inf.datetime, p->inf.datetime) > 0){
			new->next = succ;
			pre->next = new;
			pthread_mutex_unlock(&t->mutex);
			return 1;
		}
	}

	pthread_mutex_unlock(&t->mutex);	
	return 0;
}

int findSlot(struct tavolo_sv* t, struct prenotazione_sv pre){
	struct pre_list* tmp;

	if(t->inf.n_posti < pre.inf.n_persone)
		return 0;
	
	pthread_mutex_lock(&t->mutex);

	for(tmp=t->prenotazioni; tmp != NULL; tmp=tmp->next){
		if(difftime(tmp->prenotazione->inf.datetime, pre.inf.datetime)==0){
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

int connectTable(int sd, struct tavolo_sv** t){
	int i; /* Indice */

	for(i=0; i<N_TAVOLI; i++){
		pthread_mutex_lock(&tavoli[i].mutex);
		if(tavoli[i].sd == -1){
			tavoli[i].sd = sd;
			*t = &tavoli[i];
			pthread_mutex_unlock(&tavoli[i].mutex);
			return 0;
		}else{
			pthread_mutex_unlock(&tavoli[i].mutex);
		}
	}

	return -1;
}

int getTable(int sd, struct tavolo_sv** t){
	int i; /* Indice */

	for(i=0; i<N_TAVOLI; i++){
		pthread_mutex_lock(&tavoli[i].mutex);
		if(tavoli[i].sd == sd){
			*t = &tavoli[i];
			pthread_mutex_unlock(&tavoli[i].mutex);
			return 0;
		}else{
			pthread_mutex_unlock(&tavoli[i].mutex);
		}
	}

	return -1;
}

int disconnectTable(int sd){
	int i; /* Indice */

	for(i=0; i<N_TAVOLI; i++){
		pthread_mutex_lock(&tavoli[i].mutex);
		if(tavoli[i].sd == sd){
			tavoli[i].sd = -1;
			pthread_mutex_unlock(&tavoli[i].mutex);
			return 0;
		}else{
			pthread_mutex_unlock(&tavoli[i].mutex);
		}
	}

	return -1;
}