#include <sys/socket.h>

#include "header.h"
#include "../server.h"

void cl_book(int sd){
	struct pre_sosp* p_sosp = NULL; /* Prenotazione in sospeso */
	len choice; /* Tavolo scelto dall'utente */
	response r; /* Risposta da inviare al client per la conferma della registrazione */

	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	p_sosp = findPrenotazioneSospesa(sd); /* Ricerca prenotazione in sospeso */
	

	/* Ricezione tavolo scelto */
	recv(sd, &choice, sizeof(choice), 0);
	choice = ntohl(choice);

	if(p_sosp == NULL){
		/* Nessuna prenotazione in sospeso */
		strcpy(r, "È necessario prima effettuare una ricerca di un tavolo libero.");
	}else{
		if(choice >= p_sosp->nlen){
			/* Tavolo scelto non valido */
			strcpy(r, "Tavolo scelto non valido.");
		}else{

			p_sosp->p->tavolo = p_sosp->t[choice];
			p_sosp->p->timestamp = time(NULL);
			
			/* Prenotazione tavolo */
			if(bookSlot(p_sosp->t[choice], p_sosp->p)){
				strcpy(r, "Prenotazione effettuata con successo.");
				printf("Prenotazione di %d posti del tavolo %d a nome %s per il %.24s effettuata per il client %s:%d\n", p_sosp->p->n_persone, p_sosp->p->tavolo->id, p_sosp->p->cognome, ctime(&p_sosp->p->datetime), inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
			
				/* Rimozione prenotazione in sospeso */
				removePrenotazioneSospesa(sd);
			}else{
				strcpy(r, "Tavolo già prenotato.");
			}
		}
	}
	send(sd, r, sizeof(r), 0);
}