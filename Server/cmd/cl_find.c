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
	memset(p_sosp, 0, sizeof(struct pre_sosp)); /* Pulizia struttura */
	p_sosp->p = malloc(sizeof(struct prenotazione_sv)); /* Allocazione memoria */
	memset(p_sosp->p, 0, sizeof(struct prenotazione_sv));  /* Pulizia struttura */
	p_sosp->sd = sd; /* Salvataggio descrittore socket */
	insertPrenotazioneSospesa(p_sosp); /* Inserimento prenotazione in sospeso */

	/* Ricezione prenotazione ----------------------------------------------- */

	if(
		(ret = recv(sd, p_sosp->p->inf.cognome, sizeof(p_sosp->p->inf.cognome), 0)) <= 0 ||
		(ret = recv(sd, &p_sosp->p->inf.n_persone, sizeof(p_sosp->p->inf.n_persone), 0)) <= 0 ||
		(ret = recv(sd, &p_sosp->p->inf.datetime, sizeof(p_sosp->p->inf.datetime), 0)) <= 0
	){
		if(ret<0) perror("cl_find");
		return -1;
	}
	p_sosp->p->inf.n_persone = ntohs(p_sosp->p->inf.n_persone);
	p_sosp->p->inf.datetime = ntohl(p_sosp->p->inf.datetime);

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
	if(send(sd, &tmp, sizeof(tmp), 0) < 0){
		perror("cl_find");
		return -1;
	}
	for(i=0; i<(int)p_sosp->nlen; i++){
		/* Invio tavoli liberi trovati */
		if(
			send(sd, &p_sosp->t[i]->inf.id, sizeof(p_sosp->t[i]->inf.id), 0) < 0 ||
			send(sd, &p_sosp->t[i]->inf.sala, sizeof(p_sosp->t[i]->inf.sala), 0) < 0 ||
			send(sd, p_sosp->t[i]->inf.ubicazione, sizeof(p_sosp->t[i]->inf.ubicazione), 0) < 0
		){
			perror("cl_find");
			return -1;
		}
	}
	return 0;
}