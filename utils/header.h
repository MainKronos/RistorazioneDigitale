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
typedef uint32_t len; 

/* Risposta del server */
typedef char response[1024]; 

typedef uint32_t tavolo_id;

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

/*--- COMANDI ------------------------------------------------------------*/

/* Comando ping */
const cmd PING = "ping"; 

/* Richiesta dell'ID del tavolo dal TableDevice */
const cmd TD_GETID = "getid"; 

/* Richiesta menu dal TableDevice */
const cmd TD_MENU = "menu";

/* Ricerca disponibilità prenotazione per il Client */
const cmd CL_FIND = "find";

/* Richiesta prenotazione tavolo per il Client */
const cmd CL_BOOK = "book";

#endif