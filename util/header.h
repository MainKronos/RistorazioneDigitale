#ifndef my_header
#define my_header

/* Tipo del piatto */
typedef char type[3];

struct piatto{
	type tipo; /* Tipo di piatto */
	char nome[255]; /* Nome del piatto */
	u_int32_t prezzo; /* Prezzo del piatto */
};

#define M_LEN 8 /* Numero di piatti del menu */

/* Comando */
typedef char cmd[20];

/* Comando ping */
const cmd PING = "ping"; 

/* Richiesta menu dal TableDevice */
const cmd TD_MENU = "menu";
#endif