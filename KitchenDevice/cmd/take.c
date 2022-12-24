#include <stdlib.h>

#include "../main.h"

int take(int sd){
	int ret; /* Valore di ritorno */
	struct comanda* new_com; /* Nuova comanda presa */
	len nlen; /* Numero piatti */
	int i; /* indice */
	tavolo_id tid; /* Numero di tavolo */

	/* Invio comando al server */
	if(send(sd, KD_TAKE, sizeof(cmd), 0) < 0){
		perror("Errore in fase di invio");
		return -1;
	}

	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Ricezione id tavolo */
	if((ret = read(sd, &tid, sizeof(tavolo_id))) <= 0){
		if(ret < 0) perror("Errore in fase di lettura");
		return -1;
	}
	tid = ntohl(tid);

	if((int)tid == -1){
		/* Non ci sono comande da preparare */
		printf("Nessuna comanda da preparare\n");
		printf("\nPremi invio per continuare...");
		getchar();
	}else{

		new_com = malloc(sizeof(struct comanda));
		memset(new_com, 0, sizeof(struct comanda));

		new_com->inf.tid = tid;

		/* Ricezione id comanda */
		if((ret = read(sd, &new_com->inf.num, sizeof(num_com))) <= 0){
			if(ret < 0) perror("Errore in fase di lettura");
			return -1;
		}
		new_com->inf.num = ntohs(new_com->inf.num);

		printf("Com%d T%d\n", new_com->inf.num, new_com->inf.tid);

		/* Ricezione numero piatti */
		if((ret = read(sd, &nlen, sizeof(len))) <= 0){
			if(ret < 0) perror("Errore in fase di lettura");
			return -1;
		}
		nlen = ntohl(nlen);

		new_com->nlen = nlen;
		new_com->p = malloc(nlen * sizeof(type));
		new_com->q = malloc(nlen * sizeof(len));

		/* Ricezione piatti e quantità */

		for(i=0; i<nlen; i++){
			if(
				(ret = read(sd, &new_com->p[i], sizeof(type))) <= 0 ||
				(ret = read(sd, &new_com->q[i], sizeof(len))) <= 0
			){
				if(ret < 0) perror("Errore in fase di lettura");
				return -1;
			}
			new_com->q[i] = ntohl(new_com->q[i]);	
			printf("%s %d\n", new_com->p[i], new_com->q[i]);
		}

		/* Aggiunta comanda alla lista */
		lAppend((void**)&comande, new_com);

		printf("\nPremi invio per continuare...");
		getchar();
	}

	return 0;
}