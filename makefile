PORT = 32000
CC = gcc

server:
	$(CC) server.c -o server

run:
	./server $(PORT)
