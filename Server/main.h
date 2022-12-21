#ifndef server_header
#define server_header

#include "header.h"

/* --- COSTANTI ------------------------------------------------------------------------- */

#define N_PIATTI 8 /* Numero totale di piatti */
#define N_TAVOLI 10 /* Numero totale di tavoli */

/* --- Strutture ------------------------------------------------------------------------ */

/* Informazioni sul tavolo del SERVER */
struct tavolo_sv {
	struct tavolo inf; /* Informazioni tavolo */

	/* Informazioni aggiuntive del server */

	int sd; /* Socket del TableDevice collegato al tavolo */
	struct pre_list* prenotazioni; /* Lista delle prenotazioni di questo tavolo */
	pthread_mutex_t mutex; /* Mutex per la gestione della lista delle prenotazioni */
};

/* Lista di prenotazioni */
struct pre_list{
	struct prenotazione_sv* prenotazione;
	struct pre_list* next;
};

/* Informazioni sulla prenotazione del SERVER */
struct prenotazione_sv{
	struct prenotazione inf; /* Informazioni prenotazione */

	/* Informazioni aggiuntive del server */

	unlock_code code; /* Codice di sblocco del tavolo */
	struct tavolo_sv* tavolo; /* Tavolo prenotato */
	time_t timestamp; /* Timestamp del momento della finalizzazione della prenotazione */
};

/* Prenotazione in sospeso */
struct pre_sosp{
	int sd; /* Socket del Client collegato alla prenotazione in sospeso */
	struct prenotazione_sv* p; /* Prenotazione da ricevere */
	struct tavolo_sv* t[N_TAVOLI]; /* Tavoli liberi trovati */
	len nlen; /* numero di tavoli liberi trovati */
	struct pre_sosp* next; /* Puntatore al prossimo elemento */
};

/* informazioni comanda */
struct comanda{
	len nlen; /* Numero di piatti */
	struct piatto* p[N_PIATTI]; /* Vettore di puntatori ai piatti */
	len q[N_PIATTI]; /* Vettore delle quantità */
	struct tavolo_sv* t; /* Puntatore al tavolo di provenienza della comanda */

	struct comanda* next; /* Puntatore al prossimo elemento */
};

/* --- Variabili Globali --------------------------------------------------------------- */

struct piatto menu[N_PIATTI]; /* Menu */
struct tavolo_sv tavoli[N_TAVOLI]; /* Tavoli */

struct comanda* comande; /* Lista delle comande */
pthread_mutex_t mutex_comande; /* Mutex per la gestione della lista delle comande */

struct pre_sosp* prenotazioni_sospese; /* Prenotazioni in sospeso */
pthread_mutex_t mutex_prenotazioni_sospese; /* Mutex per la gestione delle prenotazioni in sospeso */


/* --- Comandi Server -------------------------------------------------------------------- */

const cmd SV_STOP = "stop"; /* Termina il server */

/* --- Gestione Comandi ------------------------------------------------------------------ */

/* Risponde al ping */
int ping(int);

/* Invia l'id del tavolo al TableDevice */
int td_getid(int);

/* Sblocca il tavolo */
int td_unlock(int);

/* Invia il menù al TableDevice */
int td_menu(int);

/* Receve la comanda dal TableDevice */
int td_comanda(int);

/* Invia le disponibilità dei tavoli al Client*/
int cl_find(int);

/* Prenota un tavolo */
int cl_book(int);

/* --- Funzioni di supporto ------------------- */

/* Inizializza il menù */
void initMenu(void);

/* Inizializza i tavoli */
void initTavoli(void);

/* Aggiunge una prenotazione al tavolo in ordine temporale */
int bookSlot(struct tavolo_sv*, struct prenotazione_sv*);

/* Cerca se è disponibile un posto per un tavolo per una prenotazione */
int findSlot(struct tavolo_sv*, struct prenotazione_sv);

/* Ricerca prenotazione in sospeso, se ritorna NULL vuol dire che non ha trovato nulla */
struct pre_sosp* findPrenotazioneSospesa(int);

/* Rimozione prenotazione in sospeso */
void removePrenotazioneSospesa(int);

/* Inserimento prenotazione in sospeso */
void insertPrenotazioneSospesa(struct pre_sosp*);

/* Connette il socket del TableDevice ad un tavolo libero, 
se ne viene trovato uno libero associa il socket al tavolo 
e ritorna 0, altrimenti -1 */
int connectTable(int, struct tavolo_sv**);

/* Disconnette il socket del TableDevice dal tavolo */
int disconnectTable(int);

/* Ritorna il tavolo associato al socket del TableDevice */
int getTable(int, struct tavolo_sv**);

/* --- COMANDI ---------------------------------------------------------------------------------------------------- */

#include "cmd/ping.c"
#include "cmd/td_getid.c"
#include "cmd/td_unlock.c"
#include "cmd/td_menu.c"
#include "cmd/td_comanda.c"
#include "cmd/cl_book.c"
#include "cmd/cl_find.c"

/* --- FUNZIONI DI SUPPORTO --------------------------------------------------------------------------------------- */

#include "utils.c"

#endif