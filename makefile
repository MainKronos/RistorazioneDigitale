
# VARIABLES
CC := gcc
CFLAGS := -Iutils -fdiagnostics-color=always -lpthread -pthread -std=c89 -pedantic -Wall -Wextra -Werror -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -ggdb

all: clean Client Server TableDevice KitchenDevice

Client: Client/main.o 
	cc $(CFLAGS) -g $< -o cli

Server: Server/main.o 
	cc $(CFLAGS) -g $< -o server 

TableDevice: TableDevice/main.o 
	cc $(CFLAGS) -g $< -o td

KitchenDevice: KitchenDevice/main.o 
	cc $(CFLAGS) -g $< -o kd

.PHONY: clean
clean:
	find . -name '*.o' -delete
