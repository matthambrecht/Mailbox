CC=cc
CFLAGS=-g -Wall
reciever:
	$(CC) $(CFLAGS) reciever.c -o reciever

sender:
	$(CC) $(CFLAGS) sender.c -o sender

all: reciever sender

clean:
	rm sender reciever
	rm /dev/shm/sample_mb
