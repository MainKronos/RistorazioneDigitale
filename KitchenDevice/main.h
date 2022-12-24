#ifndef kitchendevice_header
#define kitchendevice_header

#include "cardinal.c"

/* --- COSTANTI ------------------------------------------------------------------------- */

len n_com; /* Numero di comande in attesa */
struct comanda* comande; /* Lista comande */

/* --- Comandi ------------------------------------------------------------------- */

/* Riceve dal server il numero di comande in attesa */
int getcomlen(int);

/* Accetta la comanda nello stato di attesa da più tempo */
int take(int);

/* Visualizza l’elenco delle comande accettate dal kitchen device */
int show(void);

/* Imposta allo stato "in servizio" la comanda com */
int ready(int);

/* --- Utility --------------------------------------------------------------------- */

/* Aggiurna il numero di comande in attesa */
int uptNumComande(int);

/* Confronta una comanda con una comanda_lite */
int cmpComandaLite(const struct comanda_lite*, const struct comanda*);

#include "cmd/getcomlen.c"
#include "cmd/take.c"
#include "cmd/show.c"
#include "cmd/ready.c"

#include "utils.c"

#endif