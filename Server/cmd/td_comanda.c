#include "header.h"
#include "../main.h"

#include <stdlib.h>
#include <string.h>

int td_comanda(int sd){
	response r; /* Risposta del server */

	struct tavolo_sv* t; /* Tavolo */
	struct comanda* new_com; /* Nuova comanda */

	memset(r, 0, sizeof(response)); /* Pulizia risposta */
	new_com = NULL;

	if(getTable(sd, &t)){
		/* Tavolo non riconosciuto */

		strcpy(r, "Il tavolo non è stato riconosciuto");
	}else{
		/* Tavolo riconosciuto */
	
		type code; /* variabile temporanea per il codice del piatto */
		int ret; /* Valore di ritorno */
		int i; /* Indice */
		int j; /* Indice */

		ret = 0;

		new_com = malloc(sizeof(struct comanda));
		memset(new_com, 0, sizeof(struct comanda)); /* Pulizia struttura */

		new_com->t = t; /* Tavolo di provenienza della comanda */
		
		/* Ricezione numero elementi */
		if((ret = recv(sd, &new_com->nlen, sizeof(new_com->nlen), 0)) <= 0){
			if(ret<0) perror("td_comanda");
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
		/* Se tutti i codici dei piatti sono stati rovati */
		if(ret != -1){ 

			/* Inserimento comanda nella lista */
			pthread_mutex_lock(&mutex_comande);
			if(comande == NULL){
				comande = new_com;
			}else{
				struct comanda* tmp;
				for(tmp = comande; tmp->next != NULL; tmp = tmp->next);
				tmp->next = new_com;
			}
			pthread_mutex_unlock(&mutex_comande);

			strcpy(r, "Comanda ricevuta");
			printf("Comanda ricevuta dal tavolo T%d di %d piatti\n", t->inf.id, new_com->nlen);
		}
	}

	send(sd, r, sizeof(response), 0); /* Invio risposta */

	return 0;
}