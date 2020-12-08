# Makefile
CC = gcc  
 
 main:
	$(CC) -pthread server.c -o server
	$(CC) -pthread client.c -o client	
