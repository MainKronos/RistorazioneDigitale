#include "../main.h"

int conto(int sd){
	int ret; /* Valore di ritorno */
	tavolo_id tmp; /* Variabile temporanea per l'invio */
	len nlen; /* Numero di piatti */
	type p; /* codice dei piatto */
	len q; /* quantità piatto */
	valore v; /* valore piatto */
	valore v_tot; /* valore totale */
	int i; /* Indice */

	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Invio comando */
	if(send(sd, TD_CONTO, sizeof(TD_CONTO), 0) < 0){
		perror("conto");
		return -1;
	}

	/* Invio id tavolo */
	tmp = htonl(TID);
	if(send(sd, &tmp, sizeof(tmp), 0) < 0){
		perror("conto");
		return -1;
	}

	/* Ricezione numero elementi */
	if((ret = recv(sd, &nlen, sizeof(nlen), 0)) <= 0){
		if(ret<0) perror("conto");
		return -1;
	}
	nlen = ntohl(nlen);

	if(nlen < 0){
		/* Alcuni piatti non sono stati serviti */
		printf("Alcuni piatti non sono ancora stati serviti.\n");
	}else if(nlen == 0){
		printf("Nessun piatto ordinato.\n");
	}else{
		/* Ricezione piatti */
		v_tot = 0;
		
		for(i=0; i<(int)nlen; i++){
			if(
				(ret = recv(sd, p, sizeof(type), 0)) <= 0 ||
				(ret = recv(sd, &q, sizeof(len), 0)) <= 0 ||
				(ret = recv(sd, &v, sizeof(valore), 0)) <= 0
			){
				if(ret<0) perror("conto");
				return -1;
			}
			q = ntohl(q);
			v = ntohl(v);

			printf("%s %d %d\n", p, q, v);
			v_tot += v;
		}

		printf("Totale: %d\n", v_tot);
		lock = 1; /* Blocco il tavolo */
		NC = 0; /* Resetto il contatore delle comande */
	}

	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}