#include "../main.h"

int kd_take(int sd){
	struct comanda_sv* com; /* Comanda da inviare */
	tavolo_id tid; /* Numero tavolo per l'invio */
	num_com n_com; /* Numero comanda per l'invio */
	len q; /* Quantità per l'invio */
	len nlen; /* Numero piatti per l'invio */
	int i; /* indice */
	enum stato_com stato; /* Stato comanda */

	stato = ATTESA;

	pthread_mutex_lock(&mutex_comande);

	/* Trovo la comanda più vecchia */

	com = lFind((void**)&comande, &stato, (cmpFun)cmpComandaStato);
	if(com) com->stato = PREPARAZIONE;

	pthread_mutex_unlock(&mutex_comande);

	if(com != NULL){

		/* Invio il numero di tavolo */
		tid = htonl(com->t->inf.id);
		send(sd, &tid, sizeof(tavolo_id), 0);

		/* Invio il numero di comanda */
		n_com = htons(com->num);
		send(sd, &n_com, sizeof(num_com), 0);

		/* Invio il numero di piatti */
		nlen = htonl(com->nlen);
		send(sd, &nlen, sizeof(len), 0);

		/* Invio piatti e quantità */
		for(i=0; i<com->nlen; i++){
			send(sd, com->p[i]->code, sizeof(type), 0);
			q = htonl(com->q[i]);
			send(sd, &q, sizeof(len), 0);
		}

		notificaCucine(-1); /* Notifica cucine (-1 = una comanda in meno) */

		notificaTavolo(com); /* Notifica tavolo */
	}else{
		tavolo_id tid; /* Numero tavolo per l'invio */

		tid = htonl(-1); /* -1 = nessun tavolo */

		/* Non ci sono comande da preparare */
		send(sd, &tid, sizeof(tavolo_id), 0);
	}
	
	return 0;
}