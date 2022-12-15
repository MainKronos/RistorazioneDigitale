
# VARIABLES
CC = gcc
CFLAGS = -Iutil -fdiagnostics-color=always -lpthread -std=c89 -ansi -pedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition

.DELETE_ON_ERROR:
all: Client

Client: Client/main.o 
	cc $(CFLAGS) $< -o cli

Server: Server/main.o 
	cc $(CFLAGS) $< -o server 

.PHONY: clean
clean:
	rm -f *.o
