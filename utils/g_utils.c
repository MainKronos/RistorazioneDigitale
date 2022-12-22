#include "./header.h"

/* Aggiunge un elemento alla lista */
int append(struct list** head, struct list* elem){
	struct list* tmp;
	
	if(*head == NULL){
		*head = elem;
		return 1;
	}
	
	for(tmp = *head; tmp->next != NULL; tmp = tmp->next);
	tmp->next = elem;
	return 1;
}