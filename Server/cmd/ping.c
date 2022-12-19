#include <sys/socket.h>

int ping(int sd){
	char buffer[] = "pong";
	send(sd, (void*)buffer, sizeof(buffer), 0);
	return 0;
}