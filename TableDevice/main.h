#ifndef tabledevice_header
#define tabledevice_header

#include "cardinal.c"

/* --- COSTANTI ------------------------------------------------------------------------- */

tavolo_id TID; /* Identificativo del tavolo */
num_com NC; /* Numero delle comande inviate */

int lock; /* Blocco del tavolo 1=bloccato, 0=sbloccato */

response notifica; /* Messaggio di notifica */

/* --- Strutture ------------------------------------------------------------------- */

/* Lista di piatti della comanda usato per il parsing da input */
struct p_com{
	struct p_com* next; /* puntatore al prossimo piatto */
	type code; /* codice del piatto */
	len num; /* quantità del piatto */
};

/* --- Comandi ------------------------------------------------------------------- */

/* Connette il dispositivo ad un tavolo */
int connecttable(int);

/* Sblocca il dispositivo */
int unlocktable(int, int*, unlock_code);

/* Mostra i dettagli dei comandi */
int help(void);

/* Mostra il menu */
int menu(int);

/* Invia una comanda al server */
int comanda(int);

/* Riceve aggiornamenti delle comande dal server */
int uptcom(int);

/* Chiede il conto */
int conto(int);

/* --- Utility --------------------------------------------------------------------- */

/* Compara 2 piatti, se hanno lo stesso codice allora sono uguali */
int cmpPiatto(struct p_com*, struct p_com*);

/* Aggiunge un piatto alla lista comanda */
int addPiattoToComanda(struct p_com**, type, len);

/* Converte la lista comanda in una comanda */
int p_comToComanda(struct p_com*, struct comanda*);

/* Stampa la notifica */
int printNotifica(void);

#include "cmd/connecttable.c"
#include "cmd/unlocktable.c"
#include "cmd/help.c"
#include "cmd/menu.c"
#include "cmd/comanda.c"
#include "cmd/uptcom.c"
#include "cmd/conto.c"

#include "utils.c"

#endif