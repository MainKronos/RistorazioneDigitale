#ifndef server_header
#define server_header

#include "header.h"

/* --- COSTANTI ------------------------------------------------------------------------- */

#define N_PIATTI 8 /* Numero massimo di piatti */
#define N_TAVOLI 10 /* Numero massimo di tavoli */

/* --- Strutture ------------------------------------------------------------------------ */

/* Prenotazione in sospeso */
struct pre_sosp{
	int sd; /* Descrittore socket */
	struct prenotazione* p; /* Prenotazione da ricevere */
	struct tavolo* t[N_TAVOLI]; /* Tavoli liberi trovati */
	len nlen; /* numero di tavoli liberi trovati */
	struct pre_sosp* next; /* Puntatore al prossimo elemento */
};

/* Stato */
enum state {
	OFF, /* Spento */
	ON /* Acceso */
};

/* --- Variabili Globali --------------------------------------------------------------- */

struct piatto menu[N_PIATTI]; /* Menu */
struct tavolo tavoli[N_TAVOLI]; /* Tavoli */

struct pre_sosp* prenotazioni_sospese; /* Prenotazioni in sospeso */
pthread_mutex_t mutex_prenotazioni_sospese; /* Mutex per la gestione delle prenotazioni in sospeso */

enum state server; /* Stato del server */

/* --- Comandi Server -------------------------------------------------------------------- */

const cmd SV_STOP = "stop"; /* Termina il server */

/* --- Gestione Comandi ------------------------------------------------------------------ */

/* Risponde al ping */
int ping(int sd);

/* Invia il menù al TableDevice */
int td_menu(int sd);

/* Invia le disponibilità dei tavoli al Client*/
int cl_find(int sd);

/* Prenota un tavolo */
int cl_book(int sd);

/* --- Funzioni di supporto ------------------- */

/* Inizializza il menù */
void initMenu(void);

/* Inizializza i tavoli */
void initTavoli(void);

/* Aggiunge una prenotazione al tavolo in ordine temporale */
int bookSlot(struct tavolo*, struct prenotazione*);

/* Cerca se è disponibile un posto per un tavolo per una prenotazione */
int findSlot(struct tavolo*, struct prenotazione);

/* Ricerca prenotazione in sospeso */
struct pre_sosp* findPrenotazioneSospesa(int);

/* Rimozione prenotazione in sospeso */
void removePrenotazioneSospesa(int);

/* Inserimento prenotazione in sospeso */
void insertPrenotazioneSospesa(struct pre_sosp*);

/* --- COMANDI ---------------------------------------------------------------------------------------------------- */

#include "cmd/ping.c"
#include "cmd/td_menu.c"
#include "cmd/cl_book.c"
#include "cmd/cl_find.c"

/* --- FUNZIONI DI SUPPORTO --------------------------------------------------------------------------------------- */

#include "utils.c"

#endif