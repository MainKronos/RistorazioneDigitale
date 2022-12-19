#include <sys/socket.h>

#include "header.h"
#include "../main.h"

int cl_find(int sd){
	struct pre_sosp* p_sosp = NULL; /* Prenotazione in sospeso */
	len tmp; /* Variabile temporanea per il trasferimento*/
	int i; /* Indice */
	int ret; /* Valore di ritorno */

	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	p_sosp = findPrenotazioneSospesa(sd); /* Ricerca prenotazione in sospeso */
	
	/* Ricerca tavolo libero */

	/* Se è stata trovata una prenotazione in sospeso, viene rimossa */
	if(p_sosp){
		free(p_sosp->p);
		free(p_sosp);
	}
	/* Allocazione di una nuova prenotazione in sospeso */
	p_sosp = malloc(sizeof(struct pre_sosp)); /* Allocazione memoria */
	memset(p_sosp, 0, sizeof(*p_sosp)); /* Pulizia struttura */
	p_sosp->p = malloc(sizeof(struct prenotazione)); /* Allocazione memoria */
	memset(p_sosp->p, 0, sizeof(*p_sosp->p));  /* Pulizia struttura */
	p_sosp->sd = sd; /* Salvataggio descrittore socket */
	insertPrenotazioneSospesa(p_sosp); /* Inserimento prenotazione in sospeso */

	/* Ricezione prenotazione ----------------------------------------------- */
	ret = recv(sd, p_sosp->p->cognome, sizeof(p_sosp->p->cognome), 0);
	if(ret<=0){
		if(ret<0) perror("cl_find: ");
		return -1;
	}
	ret = recv(sd, &p_sosp->p->n_persone, sizeof(p_sosp->p->n_persone), 0);
	if(ret<=0){
		if(ret<0) perror("cl_find: ");
		return -1;
	}
	p_sosp->p->n_persone = ntohs(p_sosp->p->n_persone);
	recv(sd, &p_sosp->p->datetime, sizeof(p_sosp->p->datetime), 0);
	if(ret<=0){
		if(ret<0) perror("cl_find: ");
		return -1;
	}
	p_sosp->p->datetime = ntohl(p_sosp->p->datetime);

	/* ---------------------------------------------------------------------- */

	/* Ricerca tavoli liberi */
	p_sosp->nlen = 0;
	for(i=0; i<N_TAVOLI; i++){
		if(findSlot(&tavoli[i], *p_sosp->p)){
			p_sosp->t[p_sosp->nlen++] = &tavoli[i];
		}
	}

	printf("Trovati %d tavoli liberi per il client %s:%d\n", p_sosp->nlen, inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

	/* Invio numero di tavoli liberi trovati */
	tmp = htons(p_sosp->nlen);
	send(sd, &tmp, sizeof(tmp), 0);
	for(i=0; i<(int)p_sosp->nlen; i++){
		/* Invio tavoli liberi trovati */
		send(sd, &p_sosp->t[i]->id, sizeof(p_sosp->t[i]->id), 0);
		send(sd, &p_sosp->t[i]->sala, sizeof(p_sosp->t[i]->sala), 0);
		send(sd, p_sosp->t[i]->ubicazione, sizeof(p_sosp->t[i]->ubicazione), 0);
	}
	return 0;
}