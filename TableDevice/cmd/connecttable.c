#include "../main.h"

int connecttable(int sd){
	int ret; /* Valore di ritorno */
	tavolo_id tmp; /* Variabile temporanea */

	/* Invio richiesta id del tavolo */
	if(send(sd, TD_GETID, sizeof(TD_GETID), 0) < 0) { 
		perror("Errore in fase di richiesta dell'id del tavolo");
		return -1;
	}

	/* Ricezione id del tavolo */
	if((ret = recv(sd, &tmp, sizeof(tmp), 0)) <= 0){
		if(ret<0) perror("Errore in fase di ricezione dell'id del tavolo");
		return -1;
	}

	tmp = ntohl(tmp);

	/* Se l'ID del tavolo è -1 significa che non ci sono più tavoli liberi da associare */
	if((int)tmp == -1){
		return -1;
	}

	TID = tmp;

	return 0;
}