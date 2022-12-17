#ifndef my_header
#define my_header
struct piatto{
	char nome[20];
	u_int8_t prezzo; /* Prezzo sicuramente inferiore a 255 euro */
};

struct menu{
	struct piatto antipasto[2];
	struct piatto primo[2];
	struct piatto secondo[2];
	struct piatto dolce[2];
};

typedef char cmd[20];

/* Comando ping */
const cmd PING = "ping"; 

/* Richiesta menu dal TableDevice */
const cmd TD_MENU = "menu";
#endif