#include <arpa/inet.h>

/* Risponde al ping */
void ping(int sock);

void ping(int sock){
	char buffer[] = "pong";
	send(sock, (void*)buffer, sizeof(buffer), 0);
}