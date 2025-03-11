#ifndef MAILBOX_H
#define MAILBOX_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_MAIL 1024

struct Mailbox {
    sem_t sem;
    size_t size;
    char buff[MAX_MAIL];
};

void check_err(int fail, const char * e_msg) {
    if (fail) {
        perror(e_msg);
        exit(EXIT_FAILURE);
    }
}

struct Mailbox * connect(char * key) { // connect to existing
    unsigned int fd;
    struct Mailbox * mb;

    fd = shm_open(
        key,
        O_RDWR,
        0
    );
    

    check_err(fd == -1, "Failed to connect to existing Mailbox");

    mb = (struct Mailbox *) mmap(
        NULL,
        sizeof(*mb),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    check_err(fd == -1, "Failed to connect to existing Mailbox");

    return mb;
}

struct Mailbox * init_mailbox(char * key) {
    unsigned int fd;
    struct Mailbox * mb;

    fd = shm_open(
        key,
        O_CREAT | O_EXCL | O_RDWR,
        0600
    );

    if (fd == -1) {
        if (errno == EEXIST) { // mailbox exists
            return connect(key);
        }

        check_err(-1, "Failed to open shared memory for mailbox");
    }

    check_err(ftruncate(fd, sizeof(struct Mailbox)) == -1, "Failure sizing mailbox");

    mb = (struct Mailbox *) mmap(
        NULL,
        sizeof(*mb),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    check_err(mb == MAP_FAILED, "Couldn't map file to shared memory");
    check_err(sem_init(
        &mb->sem,
        1,
        0
    ) == -1, "Failed to initialize semaphore");

    return mb;
}

void write_mb(struct Mailbox * mb, const char * msg) {
    mb->size = strlen(msg);

    for (size_t j = 0; j < mb->size; j++) {
        mb->buff[j] = msg[j];
    }

    sem_post(&mb->sem);
}

char * read_mb(struct Mailbox * mb) {
    char buff[MAX_MAIL];

    sem_wait(&mb->sem);
    memcpy(buff, mb->buff, mb->size);
    mb->size = 0;
    sem_post(&mb->sem);

    return mb->buff;
}

#endif