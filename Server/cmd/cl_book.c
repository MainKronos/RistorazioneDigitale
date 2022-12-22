#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#include "cardinal.c"
#include "../main.h"

int cl_book(int sd){
	struct pre_sosp* p_sosp = NULL; /* Prenotazione in sospeso */
	len choice; /* Tavolo scelto dall'utente */
	response r; /* Risposta da inviare al client per la conferma della registrazione */
	int ret; /* Valore di ritorno */

	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	p_sosp = removePrenotazioneSospesa(sd);
	
	/* Ricezione tavolo scelto */
	ret = recv(sd, &choice, sizeof(choice), 0);
	if(ret<=0){
		if(ret<0) perror("cl_book");
		return -1;
	}
	choice = ntohl(choice);

	memset(r, 0, sizeof(r)); /* Pulizia buffer */

	if(p_sosp == NULL){
		/* Nessuna prenotazione in sospeso */
		strcpy(r, "È necessario prima effettuare una ricerca di un tavolo libero.");
	}else{
		if(choice >= p_sosp->nlen){
			/* Tavolo scelto non valido */
			strcpy(r, "Tavolo scelto non valido.");
		}else{
			struct prenotazione_sv* p; /* Prenotazione */

			p = p_sosp->p;

			p->code = (unlock_code)p->inf.datetime;
			p->tavolo = p_sosp->t[choice];
			p->timestamp = time(NULL);
			
			/* Prenotazione tavolo */
			if(bookSlot(p_sosp->t[choice], p)){
				char datetime[1024]; /* Buffer per il tempo */

				strftime(datetime, sizeof(datetime), "%d/%m/%Y %H:%M:%S", localtime(&p->inf.datetime));

				sprintf(r, "Prenotazione effettuata con successo.\nNome: %s\nTavolo: T%d\nNumero persone: %d\nData: %s\nCodice di sblocco: %x", p->inf.cognome, p->tavolo->inf.id, p->inf.n_persone, datetime, p->code);
				printf("Prenotazione di %d posti del tavolo %d a nome %s per il %s effettuata per il client %s:%d\n", p->inf.n_persone, p->tavolo->inf.id, p->inf.cognome, datetime, inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
			
				/* Rimozione prenotazione in sospeso */
				free(p_sosp);
			}else{
				strcpy(r, "Tavolo già prenotato.");
			}
		}
	}
	if(send(sd, r, sizeof(r), 0) < 0){
		perror("cl_book");
		return -1;
	}
	return 0;
}