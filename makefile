
# VARIABLES
CC = gcc
CFLAGS = -Iutil -fdiagnostics-color=always -lpthread -pthread -std=gnu89 -pedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -ggdb

.DELETE_ON_ERROR:
all: Client Server TableDevice

Client: Client/main.o 
	cc $(CFLAGS) -g $< -o cli

Server: Server/main.o 
	cc $(CFLAGS) -g $< -o server 

TableDevice: TableDevice/main.o 
	cc $(CFLAGS) -g $< -o td

.PHONY: clean
clean:
	rm -f *.o
