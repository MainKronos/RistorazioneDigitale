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

int cmpComandaLite(const struct comanda_lite* com, const struct comanda* com_i){
	if(com->tid == com_i->inf.tid && com->num == com_i->inf.num) return 1;
	return 0;
}