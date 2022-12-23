#include "../main.h"


int uptcom(int sd){

	/* ricevo il messaggio */
	if(recv(sd, notifica, sizeof(response), 0) < 0){
		perror("Errore in fase di ricezione");
		return -1;
	}

	return 0;
}