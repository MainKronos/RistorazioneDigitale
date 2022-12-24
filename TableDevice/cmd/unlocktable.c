#include "../main.h"

int unlocktable(int sd, int* lock, unlock_code code){
	int ret; /* Valore di ritorno */
	uint8_t r_lock; /* Variabile per la risposta di unlock */
	response res; /* Variabile per la risposta del server */
	tavolo_id tid; /* Id del tavolo per l'invio */

	printf("\033[H\033[J"); /* Pulizia schermo */
	
	/* Invio richiesta di sblocco */
	if(send(sd, TD_UNLOCK, sizeof(TD_UNLOCK), 0) < 0){ 
		perror("Errore in fase di richiesta di sblocco");
		return -1;
	}

	/* Invio id tavolo */
	tid = htonl(TID);
	if(send(sd, &tid, sizeof(tavolo_id), 0) < 0){ 
		perror("Errore in fase di invio dell'id del tavolo");
		return -1;
	}

	/* Invio codice di sblocco */
	code = htonl(code);
	if(send(sd, &code, sizeof(unlock_code), 0) < 0){ 
		perror("Errore in fase di invio del codice di sblocco");
		return -1;
	}

	/* Ricezione risposta di unlock */
	if((ret = recv(sd, &r_lock, sizeof(uint8_t), 0)) <= 0){ 
		if(ret<0) perror("Errore in fase di ricezione della risposta di sblocco");
		return -1;
	}

	if(!r_lock) *lock = 0;

	/* Ricezione risposta del server */
	if((ret = recv(sd, &res, sizeof(response), 0)) <= 0){ 
		if(ret<0) perror("Errore in fase di ricezione della risposta del server");
		return -1;
	}

	printf("%s\n", res);
	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}