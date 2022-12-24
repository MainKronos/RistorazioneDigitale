#include "../main.h"

int show(void){
	struct comanda* tmp; /* Comanda temporanea */
	int i; /* Indice */

	printf("\033[H\033[J"); /* Pulizia schermo */

	if(comande == NULL){
		printf("Nessuna comanda da accettata.\n");
	}else{
		for(tmp = comande; tmp != NULL; tmp = tmp->next){
			printf("com%hu T%d\n", tmp->inf.num, tmp->inf.tid);
			for(i=0; i<tmp->nlen; i++){
				printf("%s %d\n", tmp->p[i], tmp->q[i]);
			}
		}
	}

	printf("\nPremi invio per continuare...");
	getchar();

	return 0;
}