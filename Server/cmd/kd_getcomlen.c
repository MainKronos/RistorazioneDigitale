#include "cardinal.c"
#include "../main.h"

#include <stdlib.h>
#include <string.h>


int kd_getcomlen(int sd){
	struct cucina_sv* c; /* Cucina */
	len n_com; /* Numero comande in attesa */
	enum stato_com stato = ATTESA; /* Stato comanda */

	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	pthread_mutex_lock(&mutex_cucine);
	c = lFind((void**)&cucine, &sd, (cmpFun)cmpCucina);
	pthread_mutex_unlock(&mutex_cucine);

	if(c == NULL){
		/* Cucina non trovata */
		c = malloc(sizeof(struct cucina_sv));
		c->sd = sd;
		pthread_mutex_lock(&mutex_cucine);
		lAppend((void**)&cucine, c);
		pthread_mutex_unlock(&mutex_cucine);

		printf("Nuovo KitchenDevice %s:%d registrato\n",  inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
	}

	n_com = lCount((void**)&comande, &stato, (cmpFun)cmpComandaStato);

	/* Invio numero comande in attesa */
	n_com = htonl(n_com);
	if(send(sd, &n_com, sizeof(n_com), 0) < 0){
		perror("kd_getcomlen");
		return -1;
	}

	return 0;
}