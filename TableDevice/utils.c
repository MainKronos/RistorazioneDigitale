#include "main.h"


int cmpPiatto(struct p_com* pref, struct p_com* pi){
	return strcmp(pref->code, pi->code) == 0;
}

int addPiattoToComanda(struct p_com** head, type code, len num){

	struct p_com* new; /* elemento da aggiungere */

	struct p_com tmp; /* p_com temporanea */
	memset(&tmp, 0, sizeof(tmp)); /* Pulizia struttura */
	strcpy(tmp.code,code);

	new = lFind((void**)head, (void*)&tmp, (cmpFun)cmpPiatto);

	if(new == NULL){
		/* Aggiunta nuovo piatto */
		new = malloc(sizeof(struct p_com));
		memset(new, 0, sizeof(struct p_com)); /* Pulizia struttura */
		strcpy(new->code, code);
		new->num = num;
		lAppend((void**)head, (void*)new);
		return 0;
	}else{
		/* Aggiornamento quantità piatto */
		new->num += num;
		return -1;
	}
}

int p_comToComanda(struct p_com* com_ptr, struct comanda* comanda){
	int i; /* indice */
	struct p_com* tmp; /* puntatore temporaneo */

	comanda->p = malloc(sizeof(type)*comanda->nlen);
	comanda->q = malloc(sizeof(len)*comanda->nlen);

	for(i=0; i<(int)comanda->nlen; i++){
		strcpy(comanda->p[i], com_ptr->code);
		comanda->q[i] = com_ptr->num;
		tmp = com_ptr;
		com_ptr=com_ptr->next;
		free(tmp); /* Libero la memoria occupata dalla lista */
	}
	
	return 0;
}

int printNotifica(void){
	printf("%s\n",notifica);
	memset(notifica, 0, sizeof(notifica));
	return 0;
}