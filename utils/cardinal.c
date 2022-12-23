/*********************************************************************
 * Cardinal - File cardine del progetto                              *
 *********************************************************************/

#ifndef my_header
#define my_header

#include <stdint.h>
#include <pthread.h>

/*--- TIPI --------------------------------------------------------------*/

/* Comando */
typedef char cmd[20];

/* Codice del piatto */
typedef char type[3];

/* Numero di elementi */
typedef int32_t len; 

/* Risposta del server */
typedef char response[2048]; 

/* Identificativo del tavolo */
typedef uint32_t tavolo_id;

/* Codice di sblocco del tavolo */
typedef uint32_t unlock_code; 

/* Funzione di comparazione; 
il primo parametro è il puntatore all'elemento di riferimento,
il secondo parametro è il puntatore all'i-esimo elemento da confrontare,
Ritorna 1 = uguale, 0 = diverso 
*/
typedef int (*cmpFun)(const void*, const void*);

/* Funzione di conteggio;
il parametro è il puntatore all'elemento i-esimo,
se la funzione ritorna true, il conteggio viene incrementato*/
typedef int (*countFun)(const void*);

/* Struttura lista base */
struct list{
	struct list* next;
};

/* Informazioni sul tavolo */
struct tavolo{
	tavolo_id id; /* Identificativo del tavolo */
	uint32_t sala; /* Sala del tavolo */
	char ubicazione[255]; /* Ubicazione del tavolo */
	uint16_t n_posti; /* Numero di posti massimi del tavolo */
};

/* Informazioni del piatto */
struct piatto{
	type code; /* Codice del piatto */
	char nome[255]; /* Nome del piatto */
	uint32_t prezzo; /* Prezzo del piatto */
};

/* Informazioni sulla prenotazione */
struct prenotazione{
	char cognome[255]; /* Cognome del cliente che ha prenotato */
	uint16_t n_persone; /* Numero di persone */
	time_t datetime; /* Data e ora della prenotazione */
};

/* Informazioni sulla comanda per il trasferimento */
struct comanda{
	tavolo_id tid; /* Identificativo del tavolo */
	len nlen; /* Numero di piatti */
	type* p; /* Puntatore al vettore dei codici dei piatti */
	len* q; /* Puntatore al vettore delle quantità */
};

/*--- COMANDI ------------------------------------------------------------*/

/* Comando ping */
const cmd PING = "ping"; 

/* Richiesta dell'ID del tavolo dal TableDevice */
const cmd TD_GETID = "getid"; 

/* Richiesta di sblocco del TableDevice */
const cmd TD_UNLOCK = "unlock"; 

/* Richiesta menu dal TableDevice */
const cmd TD_MENU = "menu";

/* Richiesta comanda dal TableDevice */
const cmd TD_COMANDA = "comanda";

/* Ricerca disponibilità prenotazione per il Client */
const cmd CL_FIND = "find";

/* Richiesta prenotazione tavolo per il Client */
const cmd CL_BOOK = "book";

/* Richiesta del KitchenDevice del numero di comande in attesa */
const cmd KD_GETCOMLEN = "getcomlen";

/* Invio aggiornamento sul numero di comande in attesa dal server alla cucina */
const cmd SV_NUMCOM = "numcom";

/* Invio aggiornamento comanda dal server al tavolo */
const cmd SV_UPDCOM = "updcom";

/* --- UTILS ------------------------------------------------------------------------- */

/* Aggiunge un elemento alla lista,
il primo parametro è la testa
il secondo è l'elemento */
int lAppend(void**, void*);

/* Cerca un elemento nella lista,
il primo parametro è la testa,
il secondo è l'elemento da cercare,
il terzo è la funzione di comparazione (NULL = default),
ritorna il puntatore all'elemento trovato, NULL altrimenti */
void* lFind(void**, void*, cmpFun);

/* Rimuove un elemento dalla lista,
il primo parametro è la testa,
il secondo è l'elemento da rimuovere,
il terzo è la funzione di comparazione (NULL = default),
ritorna il puntatore all'elemento rimosso, NULL altrimenti */
void* lRemove(void**, void*, cmpFun);

/* Conta gli elementi nella lista,
il primo parametro è la testa,
il secondo è la funzione di conteggio (NULL = default),
ritorna il numero di elementi contati */
int lCount(void**, countFun);

/* Funzione di comparazione di default */
int defaultCmpFun(const void*, const void*);

/* Funzione di conteggio di default */
int defaultCountFun(const void*);

/* Aggiunge un elemento alla lista */
int lAppend(void** head, void* elem){
	struct list* tmp;
	
	if(*head == NULL){
		*head = elem;
		return 1;
	}
	
	for(tmp = *(struct list**)head; tmp->next != NULL; tmp = tmp->next);
	tmp->next = elem;
	((struct list*)elem)->next = NULL;
	return 0;
}

void* lFind(void** head, void* elem, cmpFun cmp){
	struct list* tmp;
	
	if(head == NULL)
		return NULL;
	
	if (cmp == NULL)
		cmp = defaultCmpFun;

	for(tmp = *(struct list**)head; tmp != NULL; tmp = tmp->next)
		if(cmp(elem, (void*)tmp))
			return tmp;
	
	return NULL;
}

void* lRemove(void** head, void* elem, cmpFun cmp){
	struct list* tmp;
	struct list* prev;
	
	if(head == NULL)
		return NULL;
	
	if (cmp == NULL)
		cmp = defaultCmpFun;

	for(tmp = *(struct list**)head, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next)
		if(cmp(elem, (void*)tmp)){
			if(prev == NULL)
				*head = tmp->next;
			else
				prev->next = tmp->next;
			return tmp;
		}
	
	return NULL;
}

int lCount(void** head, countFun count){
	struct list* tmp;
	int i;
	
	if(head == NULL)
		return 0;

	if (count == NULL)
		count = defaultCountFun;
	
	for(tmp = *(struct list**)head; tmp != NULL; tmp = tmp->next){
		if(count(tmp)) i++;
	}
	
	return i;
}

int defaultCountFun(const void* a){
	return a != NULL;
}

int defaultCmpFun(const void* a, const void* b){
	return a == b;
}

#endif

