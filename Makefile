CC=cc
CFLAGS=-g -Wall
client_1:
	$(CC) $(CFLAGS) client_1.c -o client_1

client_2:
	$(CC) $(CFLAGS) client_2.c -o client_2

all: client_1 client_2

clean:
	rm client_2 client_1
