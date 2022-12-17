#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "header.h"


/* Risponde al ping */
void ping(int sd);

/* Invia il menù al TableDevice */
void td_menu(int sd);

void ping(int sd){
	char buffer[] = "pong";
	send(sd, (void*)buffer, sizeof(buffer), 0);
}

void td_menu(int sd){
	static int loaded = 0; /* Se il menu è già stato caricato */
	static struct menu m; /* Menu */
	struct piatto p; /* Piatto */

	/* Controllo se il menu è stato inizzializzato, altrimenti lo inizializzo */
	if(!loaded){
		FILE* fptr;
		short unsigned tmp; /* Variabile temporanea */

		/* Apertura file */
		fptr = fopen("menu.txt", "r");
		if(fptr == NULL){
			perror("Errore in fase di apertura del file: ");
			exit(-1);
		}

		// TODO: lettura del menù non legge correttamente


		/* Antipasti */
		fscanf(fptr, "A1 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.antipasto[0] = p;
		fscanf(fptr, "A2 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.antipasto[0] = p;

		/* Primi */
		fscanf(fptr, "P1 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.primo[0] = p;
		fscanf(fptr, "P2 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.primo[1] = p;

		/* Secondi */
		fscanf(fptr, "S1 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.secondo[0] = p;
		fscanf(fptr, "S2 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.secondo[1] = p;

		/* Dolci */
		fscanf(fptr, "D1 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.dolce[0] = p;
		fscanf(fptr, "D2 %s %hu", p.nome, &tmp);
		p.prezzo = (u_int8_t)tmp;
		m.dolce[1] = p;

		fclose(fptr);
		
		loaded = 1;
	}
	
	send(sd, (void*)&m, sizeof(m), 0); /* Invio del menu */
}