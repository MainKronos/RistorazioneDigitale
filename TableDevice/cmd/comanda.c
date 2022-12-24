#include "../main.h"

int comanda(int sd){
	len n_piatti; /* numero piatti della comanda */
	struct p_com* com_ptr; /* puntatore alla lista di piatti della comanda */
	struct comanda com; /* struttura comanda per il traferimento */
	type code_tmp; /* variabile temporanea del codice del piatto */
	len num_tmp; /* variabile temporanea della quantità del piatto */
	int i; /* indice */
	char buffer[5]; /* buffer per parsare l'input dei piatti */
	char c; /* carattere temporaneo */
	int sw; /* variabile di controllo per il parser */
	response r; /* risposta del server */
	len tmp; /* variabile temporanea per l'invio */
	tavolo_id tmp_t; /* variabile temporanea per l'invio */
	num_com tmp_n; /* variabile temporanea per l'invio del numero della comanda */

	printf("\033[H\033[J"); /* Pulizia schermo */

	/*************************************************************************
	Blocco di codice per parsare l'input dei piatti.
	NON MODIFICARE ASSOLUTAMENTE .
	**************************************************************************/

	n_piatti = 0;
	com_ptr = NULL;
	i=0;
	sw = 0;
	while(1){
		c = getchar();
		buffer[i] = c;
		if(c == '-' || c == ' ' || c == '\n'){
			buffer[i] = '\0';
			if(sw){
				/* Quantità piatto */
				num_tmp = atoi(buffer);

				/* Aggiunta piatto alla lista */
				if(!addPiattoToComanda(&com_ptr, code_tmp, num_tmp))
					n_piatti++;
				sw = 0;
			}else{
				/* Codice piatto */
				if(sscanf(buffer, "%s", code_tmp) != -1) sw = 1;
			}
			i=0;
		}
		else i++;
		if(c == '\n') break;
	}

	/*****************************************************************************/

	/* Converto la lista comanda nel formato standard per il trasferimento */

	memset(&com, 0, sizeof(com)); /* Pulizia struttura */
	com.nlen = n_piatti;
	com.inf.tid = TID;
	p_comToComanda(com_ptr, &com);

	com_ptr = NULL; /* Pulizia lista comanda */


	/* Invio richiesta di invio comanda */
	if(send(sd, TD_COMANDA, sizeof(TD_COMANDA), 0) < 0){
		perror("Errore in fase di invio richiesta di invio comanda");
		return -1;
	}

	/* Invio identificativo tavolo */
	tmp_t = htonl(com.inf.tid);
	if(send(sd, &tmp_t, sizeof(tmp_t), 0) < 0){
		perror("Errore in fase di invio identificativo tavolo");
		return -1;
	}

	/* Invio il numero della comanda relativo al tavolo */
	tmp_n = htons(NC++);
	if(send(sd, &tmp_n, sizeof(tmp_n), 0) < 0){
		perror("Errore in fase di invio numero comanda");
		return -1;
	}

	/* Invio numero piatti della comanda */
	tmp = htonl(com.nlen);
	if(send(sd, &tmp, sizeof(tmp), 0) < 0){
		perror("Errore in fase di invio numero piatti");
		return -1;
	}
	for(i=0; i<(int)com.nlen; i++){
		/* Invio codice piatto e quantità*/
		
		tmp = htonl(com.q[i]);
		if(
			send(sd, com.p[i], sizeof(type), 0) < 0 ||
			send(sd, &tmp, sizeof(len), 0) < 0
		){
			perror("Errore in fase di invio codice piatto");
			return -1;
		}
	}

	free(com.p); /* Libero memoria allocata per la lista di codici */
	free(com.q); /* Libero memoria allocata per la lista di quantità */

	/* Ricezione risposta dal server */
	if(recv(sd, r, sizeof(r), 0) < 0){
		perror("Errore in fase di ricezione risposta dal server");
		return -1;
	}

	printf("%s\n", r);
	printf("\nPremi INVIO per continuare...");
	getchar();

	return 0;
}