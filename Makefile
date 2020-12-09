# Makefile
CC = gcc  
 
 main:
	$(CC) -pthread server.c -o server -Iinclude -lpthread -Llib -lSDL2 -lSDL2_ttf -lm
	$(CC) -pthread client.c -o client -Iinclude -lpthread -Llib -lSDL2 -lSDL2_ttf -lm


