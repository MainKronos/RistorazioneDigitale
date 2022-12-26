#include "../main.h"

/* Stampa tutte le comande relative ad un tavolo */
static int printComandaTavolo(const struct tavolo_sv*, const struct comanda_sv*);

/* Stampa tutte le comande relative ad uno stato */
static int printComandaStato(const enum stato_com*, const struct comanda_sv*);

int sv_stat(void){
	char c;
	enum stato_com stato; /* Stato delle comande da stampare */

	c = getchar();
	if(c == 'T'){
		tavolo_id tid; /* Id del tavolo */
		scanf("%u", &tid);

		pthread_mutex_lock(&mutex_comande);
		if(!lCount((void**)&comande, (void*)&tavoli[tid], (cmpFun)printComandaTavolo)){
			printf("Nessuna comanda per il tavolo T%u\n", tid);
		}
		pthread_mutex_unlock(&mutex_comande);

		return 0;

	}else if(c == 'a'){
		stato = ATTESA;
	}else if(c == 'p'){
		stato = PREPARAZIONE;
	}else if(c == 's'){
		stato = SERVIZIO;
	}else{
		return 0;
	}

	pthread_mutex_lock(&mutex_comande);
	if(!lCount((void**)&comande, (void*)&stato, (cmpFun)printComandaStato)){
		printf("Nessuna comanda in stato '%s'\n", stato_com_str[stato]);
	}
	pthread_mutex_unlock(&mutex_comande);

	return 0;
}

static int printComandaTavolo(const struct tavolo_sv* t, const struct comanda_sv* c_i){
	int i; /* Indice */
	if(c_i->t == t){
		printf("com%d <%s>\n", c_i->num, stato_com_str[c_i->stato]);
		for(i=0; i<c_i->nlen; i++){
			printf("%s %d\n", c_i->p[i]->code, c_i->q[i]);
		}
		return 1;
	}
	return 0;
}

static int printComandaStato(const enum stato_com* stato, const struct comanda_sv* c_i){
	int i; /* Indice */
	if(c_i->stato == *stato){
		printf("com%d T%d\n", c_i->num, c_i->t->inf.id);
		for(i=0; i<c_i->nlen; i++){
			printf("%s %d\n", c_i->p[i]->code, c_i->q[i]);
		}
		return 1;
	}
	return 0;
}