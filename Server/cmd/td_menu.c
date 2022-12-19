#include <sys/socket.h>

#include "header.h"
#include "../main.h"

int td_menu(int sd){
	len nlen; /* Variabile temporanea per il trasferimento*/
	unsigned int i; /* Indice */

	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	/* Invio il numero di elementi */
	nlen = htonl(N_PIATTI);
	send(sd, (void*)&nlen, sizeof(nlen), 0);
	
	/* Invio del menu */
	for(i=0; i<N_PIATTI; i++){
		uint32_t tmp = htonl(menu[i].prezzo);
		send(sd, (void*)&menu[i].tipo, sizeof(menu[i].tipo), 0);
		send(sd, (void*)&menu[i].nome, sizeof(menu[i].nome), 0);
		send(sd, (void*)&tmp, sizeof(tmp), 0);
	}

	printf("Menu inviato a %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

	return 0;
}