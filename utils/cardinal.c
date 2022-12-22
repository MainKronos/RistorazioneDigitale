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

/* Stato della comanda */
typedef uint8_t stato_com; 

/* Funzione di comparazione; 
il primo parametro è il puntatore all'elemento di riferimento,
il secondo parametro è il puntatore all'i-esimo elemento da confrontare,
Ritorna 1 = True, 0 = False 
*/
typedef int (*cmpFun)(struct list*, struct list*);

/* Conversione stato_com in stringa */
const char* stato_com_str[] = {
	"in attesa",
	"in preparazione",
	"in servizio"
};

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

/* Invio aggiornamento comanda dal server al tavolo */
const cmd SV_UPDCOM = "updcom";

/* --- UTILS ------------------------------------------------------------------------- */

/* Aggiunge un elemento alla lista,
il primo parametro è la testa
il secondo è l'elemento */
int append(struct list**, struct list*);

/* Cerca un elemento nella lista,
il primo parametro è la testa
il secondo è l'elemento da cercare
il terzo è la funzione di comparazione (NULL = default) */
struct list* find(struct list*, struct list*, cmpFun);

/* Funzione di comparazione di default */
int defaultCmpFun(struct list*, struct list*);

/* Aggiunge un elemento alla lista */
int append(struct list** head, struct list* elem){
	struct list* tmp;
	
	if(*head == NULL){
		*head = elem;
		return 1;
	}
	
	for(tmp = *head; tmp->next != NULL; tmp = tmp->next);
	tmp->next = elem;
	return 1;
}

struct list* find(struct list* head, struct list* elem, cmpFun cmp){
	struct list* tmp;
	
	if(head == NULL)
		return NULL;
	
	if (cmp == NULL)
		cmp = defaultCmpFun;

	for(tmp = head; tmp != NULL; tmp = tmp->next)
		if(cmp(elem, tmp))
			return tmp;
	
	return 0;
}

int defaultCmpFun(struct list* a, struct list* b){
	return a == b;
}

#endif

