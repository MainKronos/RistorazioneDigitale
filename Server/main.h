#ifndef server_header
#define server_header

#include "cardinal.c"

/* --- COSTANTI ------------------------------------------------------------------------- */

#define N_PIATTI 8 /* Numero totale di piatti */
#define N_TAVOLI 10 /* Numero totale di tavoli */

/* --- Strutture ------------------------------------------------------------------------ */

/* Informazioni sul tavolo del SERVER */
struct tavolo_sv {
	struct tavolo inf; /* Informazioni tavolo */

	/* Informazioni aggiuntive del server */

	int sd; /* Socket del TableDevice collegato al tavolo */
	struct prenotazione_sv* prenotazioni; /* Lista delle prenotazioni di questo tavolo */
	pthread_mutex_t mutex; /* Mutex per la gestione della lista delle prenotazioni */
};

/* Informazioni sul kitchen device del SERVER */
struct cucina_sv{
	struct cucina_sv* next; /* Puntatore al prossimo elemento */
	int sd; /* Socket del KitchenDevice collegato */
};

/* Informazioni sulla prenotazione del SERVER */
struct prenotazione_sv{
	struct prenotazione_sv* next; /* Puntatore al prossimo elemento */

	struct prenotazione inf; /* Informazioni prenotazione */
	/* Informazioni aggiuntive del server */

	unlock_code code; /* Codice di sblocco del tavolo */
	struct tavolo_sv* tavolo; /* Tavolo prenotato */
	time_t timestamp; /* Timestamp del momento della finalizzazione della prenotazione */
	
};

/* Prenotazione in sospeso */
struct pre_sosp{
	struct pre_sosp* next; /* Puntatore al prossimo elemento */
	int sd; /* Socket del Client collegato alla prenotazione in sospeso */
	struct prenotazione_sv* p; /* Prenotazione da ricevere */
	struct tavolo_sv* t[N_TAVOLI]; /* Tavoli liberi trovati */
	len nlen; /* numero di tavoli liberi trovati */
};

/* Stati della comanda */
enum stato_com{
	ATTESA,
	PREPARAZIONE,
	SERVIZIO
};

/* Conversione stato_com in stringa */
const char* stato_com_str[] = {
	"in attesa",
	"in preparazione",
	"in servizio"
};

/* informazioni comanda del SERVER */
struct comanda_sv{
	struct comanda_sv* next; /* Puntatore al prossimo elemento */
	enum stato_com stato; /* Stato della comanda */
	num_com num; /* Numero della comanda relativo al tavolo */
	len nlen; /* Numero di piatti */
	struct piatto* p[N_PIATTI]; /* Vettore di puntatori ai piatti */
	len q[N_PIATTI]; /* Vettore delle quantità */
	struct tavolo_sv* t; /* Puntatore al tavolo di provenienza della comanda */
};

/* Informazioni comanda da cercare, utilizzata solo per scopi di ricerca */
struct comanda_to_search{
	enum stato_com stato; /* Stato della comanda */
	struct tavolo_sv* t; /* Puntatore al tavolo di provenienza della comanda */
};

/* --- Variabili Globali --------------------------------------------------------------- */

struct piatto menu[N_PIATTI]; /* Menu */
struct tavolo_sv tavoli[N_TAVOLI]; /* Tavoli */

struct comanda_sv* comande; /* Lista delle comande del server */
pthread_mutex_t mutex_comande; /* Mutex per la gestione della lista delle comande */

struct pre_sosp* prenotazioni_sospese; /* Prenotazioni in sospeso */
pthread_mutex_t mutex_prenotazioni_sospese; /* Mutex per la gestione delle prenotazioni in sospeso */

struct cucina_sv* cucine; /* Lista delle cucine del server */
pthread_mutex_t mutex_cucine; /* Mutex per la gestione della lista delle cucine */


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

/* Riceve la richiesta di conto dal TableDevice */
int td_conto(int);

/* Invia le disponibilità dei tavoli al Client*/
int cl_find(int);

/* Prenota un tavolo */
int cl_book(int);

/* Risponde al kitchenDevice con il numero di comande in attesa */
int kd_getcomlen(int);

/* Invia la comanda al KitchenDevice */
int kd_take(int);

/* Riceve dal KitchenDevice che una comanda è pronta */
int kd_ready(int);

/* Controlla se è possibile spegnere il server */
int sv_stop(void);

/* Mostra comande */
int sv_stat(void);

/* --- Funzioni di supporto ------------------- */

/* Inizializza il menù */
void initMenu(void);

/* Inizializza i tavoli */
void initTavoli(void);

/* Aggiunge una prenotazione al tavolo in ordine temporale */
int bookSlot(struct tavolo_sv*, struct prenotazione_sv*);

/* Cerca se è disponibile un posto per un tavolo per una prenotazione */
int findSlot(struct tavolo_sv*, struct prenotazione_sv);

/* Rimozione prenotazione in sospeso */
struct pre_sosp* removePrenotazioneSospesa(int);

/* Connette il socket del TableDevice ad un tavolo libero, 
se ne viene trovato uno libero associa il socket al tavolo 
e ritorna 0, altrimenti -1 */
int connectTable(int, struct tavolo_sv**);

/* Disconnette il socket del TableDevice dal tavolo */
int disconnectTable(int);

/* Rimuove la cucina se registrata */
int removeCucina(int);

/* Confronta se il codice di prenotazione è uguale */
int cmpCodePrenotazione(const unlock_code*, const struct prenotazione_sv*);

/* Confronta se il sd è uguale di una prenotazione in sospeso */
int cmpPrenotazioneSospeso(const int*, const struct pre_sosp*);

/* Confronta se il sd è uguale di una cucina */
int cmpCucina(const int*, const struct cucina_sv*);

/* Controlla se una comanda ha uno stato */
int cmpComandaStato(const enum stato_com*, const struct comanda_sv*);

/* Notifica tutte le cucine che è avvenuta una variazione nel numero di comande in attesa */
int notificaCucine(len);

/* Notifica il tavolo che la comanda ha cambiato stato */
int notificaTavolo(struct comanda_sv*);

/* Confronta una comanda_sv con una comanda_lite */
int cmpComandaLite(const struct comanda_lite*, const struct comanda_sv*);

/* Confronta una comanda_sv con una comanda_to_search */
int cmpComandaToSearch(const struct comanda_to_search*, const struct comanda_sv*);

/* Confronta una comanda_sv con una comanda_to_search considerando lo stato diverso */
int cmpComandaToSearchNotStato(const struct comanda_to_search* com_s, const struct comanda_sv* com_i);

/* --- COMANDI ---------------------------------------------------------------------------------------------------- */

#include "cmd/ping.c"
#include "cmd/td_getid.c"
#include "cmd/td_unlock.c"
#include "cmd/td_menu.c"
#include "cmd/td_comanda.c"
#include "cmd/cl_book.c"
#include "cmd/cl_find.c"
#include "cmd/kd_getcomlen.c"
#include "cmd/kd_take.c"
#include "cmd/kd_ready.c"
#include "cmd/td_conto.c"
#include "cmd/sv_stop.c"
#include "cmd/sv_stat.c"

/* --- FUNZIONI DI SUPPORTO --------------------------------------------------------------------------------------- */

#include "utils.c"

#endif