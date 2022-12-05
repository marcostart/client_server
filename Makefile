CC=gcc
exe1 = server
exe2 = ./clientd/client


# all: ./clientd/client server clean

./clientd/client: client_prev.c
	$(CC) $< -o $@

server: server_prev.c
	$(CC) $< -o $@

run1: SHELL:=/bin/bash
run1: $(exe1) $(exe2)
	

clean:
	rm -f *.o
