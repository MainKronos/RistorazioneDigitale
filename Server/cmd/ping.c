#include <sys/socket.h>

void ping(int sd){
	char buffer[] = "pong";
	send(sd, (void*)buffer, sizeof(buffer), 0);
}