CFLAGS = -Wall -lm

client: client.c
	gcc -o client -g client.c $(CFLAGS)

server: server.c
	gcc -o server -g server.c $(CFLAGS)

all: client server

clean:
	rm -f *.o client.exe client server.exe server
