#include "../main.h"

int td_conto(int sd){
	tavolo_id tid; /* Id del tavolo */
	len nlen; /* Numero di piatti */
	len len_tmp; /* Variabile quantità per l'invio */
	valore v_tmp; /* Valore del piatto per l'invio */
	int ret; /* Valore di ritorno */
	struct comanda_sv* c_tmp; /* Comanda temporanea */
	struct comanda_to_search cts; /* Comanda da cercare */
	struct piatto* piatti[N_PIATTI]; /* Piatti da inviare */
	len q[N_PIATTI]; /* Quantità */
	int i; /* Indice */
	int j; /* Indice */

	/* Ricevo l'id del tavolo */
	if((ret = recv(sd, &tid, sizeof(tid), 0)) <= 0){
		if(ret<0) perror("td_conto");
		return -1;
	}
	tid = ntohl(tid);

	/* Cerco se esistono comanda in attesa o in preparazione */

	cts.stato = SERVIZIO;
	cts.t = &tavoli[tid];

	pthread_mutex_lock(&mutex_comande);
	c_tmp = lFind((void**)&comande, &cts, (cmpFun)cmpComandaToSearchNotStato);
	pthread_mutex_unlock(&mutex_comande);

	if(c_tmp != NULL){
		/* Invio numero di piatti */
		nlen = htonl(-1);
		if(send(sd, &nlen, sizeof(nlen), 0) < 0){
			perror("td_conto");
			return -1;
		}
	}else{
		/* Non ci sono comande in attesa o in preparazione */

		for(i=0; i<N_PIATTI; i++){
			piatti[i] = &menu[i];
		}
		memset(q, 0, sizeof(q));

		nlen=0;

		/* Trovo i piatti e aggiorno le quantità */
		pthread_mutex_lock(&mutex_comande);
		while((c_tmp = lRemove((void**)&comande, &cts, (cmpFun)cmpComandaToSearch)) != NULL){
			nlen++;
			
			for(i=0; i<c_tmp->nlen; i++){
				for(j=0; j<N_PIATTI; j++){
					if(piatti[j] == c_tmp->p[i]){
						q[j] += c_tmp->q[i];
						break;
					}
				}
			}

			free(c_tmp);
		}
		pthread_mutex_unlock(&mutex_comande);

		/* Invio numero di piatti */
		nlen = htonl(nlen);
		if(send(sd, &nlen, sizeof(nlen), 0) < 0){
			perror("td_conto");
			return -1;
		}

		/* Invio i piatti */
		for(i=0; i<N_PIATTI; i++){
			if(q[i] > 0){
				len_tmp = htonl(q[i]);
				v_tmp = htonl(piatti[i]->prezzo * q[i]);
				if(
					send(sd, piatti[i]->code, sizeof(type), 0) < 0 ||
					send(sd, &len_tmp, sizeof(len_tmp), 0) < 0 ||
					send(sd, &v_tmp, sizeof(v_tmp), 0) < 0
				){
					perror("td_conto");
					return -1;
				}
			}
		}
	}
	return 0;
}

