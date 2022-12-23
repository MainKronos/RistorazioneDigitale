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

/* --- Utility --------------------------------------------------------------------- */

/* Aggiurna il numero di comande in attesa */
int uptNumComande(int);

#include "cmd/getcomlen.c"
#include "cmd/take.c"

#include "utils.c"

#endif