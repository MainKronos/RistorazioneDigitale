#include "../main.h"

int sv_stop(void){
	enum stato_com attesa = ATTESA; /* Stato della comanda */
	enum stato_com preparazione = PREPARAZIONE; /* Stato della comanda */

	int ret; /* Valore di ritorno della funzione */

	/* Cerco se esistono comanda in attesa o in preparazione */
	pthread_mutex_lock(&mutex_comande);
	if(
		lFind((void**)&comande, &attesa, (cmpFun)cmpComandaStato) ||
		lFind((void**)&comande, &preparazione, (cmpFun)cmpComandaStato)
	){
		/* Ci sono comande in attesa o in preparazione */
		printf("Ci sono ancora comande in attesa o in preparazione.\n");
		ret = 0; /* Non posso fermare il server */
	}else{
		/* Non ci sono comande in attesa o in preparazione */
		ret = 1; /* Posso fermare il server */
	}
	pthread_mutex_unlock(&mutex_comande);

	return ret;
}