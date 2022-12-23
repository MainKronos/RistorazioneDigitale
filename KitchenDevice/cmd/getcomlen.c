#include "../main.h"

int getcomlen(int sd){
	int ret;
	len tmp;

	/* Invio richiesta */
	if(send(sd, KD_GETCOMLEN, sizeof(cmd), 0) < 0){
		perror("Errore in fase di invio");
		return -1;
	}

	if((ret = read(sd, &tmp, sizeof(len))) <= 0){
		if(ret < 0) perror("Errore in fase di lettura");
		return -1;
	}
	n_com = ntohl(tmp);

	return 0;
}