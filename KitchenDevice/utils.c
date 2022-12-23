#include "main.h"

int uptNumComande(int sd){
	int ret;
	len tmp;

	if((ret = read(sd, &tmp, sizeof(len))) <= 0){
		if(ret < 0) perror("Errore in fase di lettura");
		return -1;
	}
	n_com += ntohl(tmp);

	return 0;
}