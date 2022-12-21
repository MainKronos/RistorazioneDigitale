#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#include "header.h"
#include "../main.h"

int td_unlock(int sd){
	int ret; /* Valore di ritorno */
	uint8_t lock; /* Variabile per il lock. 1 = lock, 0 = unlock */
	unlock_code code; /* Codice di sblocco */
	struct tavolo_sv* t; /* Tavolo */
	response r; /* Risposta del server */

	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	lock = 1;
	memset(r, 0, sizeof(response)); /* Pulizia risposta */

	/* Ricezione codice */
	if((ret = recv(sd, &code, sizeof(code), 0)) <= 0){
		if(ret<0) perror("td_unlock");
		return -1;
	}
	code = ntohl(code);

	if(getTable(sd, &t)){
		/* Tavolo non riconosciuto */
		lock = 1;
		strcpy(r, "Il tavolo non è stato riconosciuto");
	}else{
		/* Tavolo riconosciuto */

		struct prenotazione_sv* p; /* Prenotazione corrispondente al codice */
		struct pre_list* p_list; /* Lista prenotazione */
		struct pre_list* p_list_pre; /* Lista prenotazione precedente a p_list */
		
		/* Cerco il codice */
		pthread_mutex_lock(&t->mutex);
		p_list_pre = NULL;
		for(p_list=t->prenotazioni; p_list!=NULL; p_list_pre=p_list, p_list=p_list->next){
			if(p_list->prenotazione->code == code){
				/* Codice trovato */
				lock = 0;
				break;
			}
		}
		if(p_list==NULL){
			/* Codice non trovato */
			lock = 1;
			strcpy(r, "Codice sblocco non valido");
		}else{
			/* Codice trovato, allora lo rimuovo dalla lista */

			char datetime[1024]; /* Buffer per il tempo */

			if(p_list_pre==NULL){
				/* Il codice è il primo della lista */
				t->prenotazioni = p_list->next;
			}else{
				/* Il codice non è il primo della lista */
				p_list_pre->next = p_list->next;
			}
			p = p_list->prenotazione;
			free(p_list);

			lock = 0;
			strftime(datetime, sizeof(datetime), "%d/%m/%Y %H:%M:%S", localtime(&p->inf.datetime));
			sprintf(r, "Tavolo sbloccato.\nNome: %s\nNumero persone: %d\nData: %s", p->inf.cognome, p->inf.n_persone, datetime);

			printf("Tavolo T%d collegato al TableDevice %s:%d sbloccato\n", p->tavolo->inf.id, inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

			free(p);
		}
		pthread_mutex_unlock(&t->mutex);

	}

	/* Invio risposta */
	if(
		send(sd, &lock, sizeof(lock), 0) < 0 ||
		send(sd, &r, sizeof(r), 0) < 0
	){
		perror("td_unlock");
		return -1;
	}
	
	return 0;
}