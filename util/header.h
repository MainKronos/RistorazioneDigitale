

#ifndef my_header
#define my_header


/*--- TIPI --------------------------------------------------------------*/

/* Comando */
typedef char cmd[20];

/* Tipo del piatto */
typedef char type[3];

typedef uint32_t len; /* Numero di elementi */

/* Informazioni del piatto */
struct piatto{
	type tipo; /* Tipo di piatto */
	char nome[255]; /* Nome del piatto */
	uint32_t prezzo; /* Prezzo del piatto */
};

/* Informazioni sulla data */
struct date{
	uint16_t giorno; /* Giorno */
	uint16_t mese; /* Mese */
	uint16_t anno; /* Anno */
};

/* Informazioni sulla prenotazione */
struct prenotazione{
	char cognome[255]; /* Cognome del cliente che ha prenotato */
	uint16_t n_persone; /* Numero di persone */
	struct date data; /* Data della prenotazione */
	uint16_t ora; /* Ora della prenotazione */

	/* Informazioni aggiuntive del server */

	struct tavolo* tavolo; /* Tavolo prenotato */
	uint64_t timestamp; /* Timestamp della prenotazione */
};

/* Informazioni sul tavolo */
struct tavolo{
	uint32_t id; /* Identificativo del tavolo */
	uint32_t sala; /* Sala del tavolo */
	char ubicazione[255]; /* Ubicazione del tavolo */

	/* Informazioni aggiuntive del server */

	uint16_t n_posti; /* Numero di posti massimi del tavolo */
	struct pre_list* prenotazioni; /* Lista delle prenotazioni di questo tavolo */
	pthread_mutex_t mutex; /* Mutex per la gestione della lista delle prenotazioni */
};

/* Lista di prenotazioni */
struct pre_list{
	struct prenotazione* prenotazione;
	struct pre_list* next;
};

/*--- COMANDI ------------------------------------------------------------*/

/* Comando ping */
const cmd PING = "ping"; 

/* Richiesta menu dal TableDevice */
const cmd TD_MENU = "menu";

/* Ricerca disponibilità prenotazione per il Client */
const cmd CL_FIND = "find";

#endif