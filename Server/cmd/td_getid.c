#include <sys/socket.h>

#include "cardinal.c"
#include "../main.h"

int td_getid(int sd){
	struct tavolo_sv* t; /* puntatore al Tavolo */
	tavolo_id id; /* Identificativo del tavolo */
	
	struct sockaddr_in cl_addr; /* Indirizzo client */
	socklen_t addrlen = sizeof(cl_addr);
	getpeername(sd, (struct sockaddr *)&cl_addr, &addrlen);

	if(connectTable(sd, &t)){
		id = -1;
	}else{
		id = t->inf.id;
		printf("TableDevice %s:%d connesso al tavolo T%d\n",  inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port), id);
	}

	/* Invio identificativo del tavolo libero trovato */
	id = htonl(id);
	if(send(sd, &id, sizeof(id), 0) < 0){
		perror("td_getid");
		return -1;
	}

	return 0;
}