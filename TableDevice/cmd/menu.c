#include "../main.h"

int menu(int sd){
	int ret; /* Valore di ritorno */
	int i; /* Indice */
	len n; /* numero piatti del menu */

	printf("\033[H\033[J"); /* Pulizia schermo */

	/* Invio richiesta del menu */
	if(send(sd, TD_MENU, sizeof(TD_MENU), 0) < 0){ 
		perror("Errore in fase di richiesta del menù");
		return -1;
	}

	/* Ricezione numero piatti del menu */
	if(recv(sd, &n, sizeof(n), 0) < 0){
		perror("Errore in fase di ricezione del menù");
		return -1;
	}
	n = ntohl(n);

	/* Ricezione menu e stampa */
	for(i=0; i<n; i++){
		struct piatto p;

		if(
			(ret = recv(sd, &p.code, sizeof(p.code), 0)) < 0 ||
			(ret = recv(sd, &p.nome, sizeof(p.nome), 0)) < 0 ||
			(ret = recv(sd, &p.prezzo, sizeof(p.prezzo), 0)) < 0
		){
			if(ret<0) perror("Errore in fase di ricezione del menù");
			return -1;
		}
		p.prezzo = ntohl(p.prezzo);

		printf("%-2s - %-30s %d\n", p.code, p.nome, p.prezzo);
	}
	
	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}