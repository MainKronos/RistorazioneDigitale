#include "../main.h"

int ready(int sd){
	struct comanda_lite com; /* Comanda lite */
	struct comanda* tmp; /* Comanda temporanea */

	scanf("com%hu-T%u", &com.num, &com.tid);
	getchar();

	printf("\033[H\033[J"); /* Pulizia schermo */

	tmp = lRemove((void**)&comande, &com, (cmpFun)cmpComandaLite);

	if(tmp == NULL){
		printf("Comanda non trovata.\n");
	}else{

		/* Invio comando */
		if(send(sd, KD_READY, sizeof(cmd), 0) < 0){
			perror("Errore in fase di invio");
			return -1;
		}

		/* Invio id tavolo e numero comanda */
		com.tid = htonl(com.tid);
		com.num = htons(com.num);
		if(
			send(sd, &com.tid, sizeof(tavolo_id), 0) < 0 ||
			send(sd, &com.num, sizeof(num_com), 0) < 0
		){
			perror("Errore in fase di invio");
			return -1;
		}

		printf("Comanda in servizio.\n");
	}

	printf("\nPremi invio per continuare...");
	getchar();

	return 0;
}