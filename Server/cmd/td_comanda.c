#include "cardinal.c"
#include "../main.h"

#include <stdlib.h>
#include <string.h>

int td_comanda(int sd){
	response r; /* Risposta del server */
	int ret; /* Valore di ritorno */
	struct tavolo_sv* t; /* Tavolo */
	tavolo_id tid; /* Id del tavolo */
	struct comanda_sv* new_com; /* Nuova comanda */

	memset(r, 0, sizeof(response)); /* Pulizia risposta */
	new_com = NULL;

	/* Ricevo l'id del tavolo */
	if((ret = recv(sd, &tid, sizeof(tid), 0)) <= 0){
		if(ret<0) perror("td_comanda");
		return -1;
	}
	tid = ntohl(tid);

	if(getTable(sd, &t) || t->inf.id != tid){
		/* Tavolo non riconosciuto */

		strcpy(r, "Il tavolo non è stato riconosciuto");
	}else{
		/* Tavolo riconosciuto */

		type code; /* variabile temporanea per il codice del piatto */
		int i; /* Indice */
		int j; /* Indice */

		ret = 0;

		new_com = malloc(sizeof(struct comanda_sv));
		memset(new_com, 0, sizeof(struct comanda_sv)); /* Pulizia struttura */

		new_com->t = t; /* Tavolo di provenienza della comanda */

		/* Ricezione numero della comanda */
		if((ret = recv(sd, &new_com->num, sizeof(num_com), 0)) <= 0){
			if(ret<0) perror("td_comanda");
			free(new_com);
			return -1;
		}
		new_com->num = ntohs(new_com->num);
		
		/* Ricezione numero elementi */
		if((ret = recv(sd, &new_com->nlen, sizeof(new_com->nlen), 0)) <= 0){
			if(ret<0) perror("td_comanda");
			free(new_com);
			return -1;
		}
		new_com->nlen = ntohl(new_com->nlen);

		/* Ricezione elementi */
		for(i=0; i<(int)new_com->nlen; i++){
			if(
				(ret = recv(sd, &code, sizeof(type), 0)) <= 0 ||
				(ret = recv(sd, &new_com->q[i], sizeof(len), 0)) <= 0
			){
				if(ret<0) perror("td_comanda");
				free(new_com);
				return -1;
			}

			new_com->q[i] = ntohl(new_com->q[i]);

			/* Conversione codice */
			for(j=0; j<N_PIATTI; j++){
				if(strcmp(menu[j].code,code) == 0){
					new_com->p[i] = &menu[j];
					break;
				}
			}
			if(j == N_PIATTI){
				/* Codice non trovato */
				sprintf(r, "Il codice del piatto %s non è stato trovato", code);
				free(new_com);
				ret = -1;
				break;
			}
		}
		/* Se tutti i codici dei piatti sono stati trovati */
		if(ret != -1){ 
			char buf1[10] = ""; /* Buffer per la stampa */
			char buf2[256] = ""; /* Buffer per la stampa */
			new_com->stato = ATTESA; /* Stato comanda: 0 = in attesa */

			/* Inserimento comanda nella lista */
			pthread_mutex_lock(&mutex_comande);

			lAppend((void**)&comande, (void*)new_com);

			pthread_mutex_unlock(&mutex_comande);

			strcpy(r, "Comanda ricevuta");
			for(i=0; i<(int)new_com->nlen; i++){
				sprintf(buf1, " %s-%d", new_com->p[i]->code, new_com->q[i]);
				strcat(buf2, buf1);
			}
			printf("Comanda ricevuta dal tavolo T%d di %d piatti:%s\n", t->inf.id, new_com->nlen, buf2);
			notificaCucine(1); /* Notifica cucine (1 = nuova comanda ricevuta) */
		}
	}

	send(sd, r, sizeof(response), 0); /* Invio risposta */

	
	
	return 0;
}