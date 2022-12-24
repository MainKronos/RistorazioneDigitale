#include "../main.h"

int kd_ready(int sd){
	struct comanda_lite com; /* Comanda lite */
	struct comanda_sv* tmp; /* Comanda_sv trovata */
	int ret; /* Valore di ritorno */

	if(
		(ret = recv(sd, &com.tid, sizeof(tavolo_id), 0)) <= 0 ||
		(ret = recv(sd, &com.num, sizeof(num_com), 0)) <= 0
	){
		if(ret<0) perror("kd_ready");
		return -1;
	}
	com.tid = ntohl(com.tid);
	com.num = ntohs(com.num);

	/* Ricerco la comanda */
	pthread_mutex_lock(&mutex_comande);

	tmp = lFind((void**)&comande, &com, (cmpFun)cmpComandaLite);
	tmp->stato = SERVIZIO;
	
	pthread_mutex_unlock(&mutex_comande);

	/* Notifico il tavolo */
	notificaTavolo(tmp);

	return 0;
}