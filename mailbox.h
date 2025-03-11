/*
mailbox.h - A Unix Shared Memory Mailing System for IPC
*/

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

#define MAX_OF 24 // Max number of officees

struct node {
    struct node * next;
    struct node * prev;
    void * content;
};

struct queue {
    size_t length;
    struct node * head;
};

struct office {
    int vacant[MAX_OF];
    sem_t sem[MAX_OF];
    char buff[MAX_OF];
    void * queue[MAX_OF];
    size_t procs;
};

void check_perr(int fail, const char * e_msg) { // for errors something else caught
    if (fail) {
        perror(e_msg);
        exit(EXIT_FAILURE);
    }
}

int check_err(int fail, const char * e_msg) { // for errors we caught
    if (fail) {
        fprintf(stderr, "%s\n", e_msg);
        return -1;
    }
}

struct office * connect(char * key, const unsigned int box_id) { // connect to existing
    unsigned int fd;
    struct office * of;

    fd = shm_open(
        key,
        O_RDWR,
        0
    );
    

    check_perr(fd == -1, "Failed to connect to existing office");

    of = (struct office *) mmap(
        NULL,
        sizeof(*of),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    check_perr(fd == -1, "Failed to connect to existing office");
    add_proc(of, box_id);

    return of;
}

struct office * init_office(char * key, const unsigned int box_id) {
    unsigned int fd;
    struct office * of;

    fd = shm_open(
        key,
        O_CREAT | O_EXCL | O_RDWR,
        0600
    );

    if (fd == -1) {
        if (errno == EEXIST) { // office exists
            return connect(key, box_id);
        }

        check_perr(-1, "Failed to open shared memory for office");
    }

    check_perr(ftruncate(fd, sizeof(struct office)) == -1, "Failure sizing office");

    of = (struct office *) mmap(
        NULL,
        sizeof(*of),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    check_perr(of == MAP_FAILED, "Couldn't map file to shared memory");
    check_perr(sem_init(
        &of->sem,
        1,
        0
    ) == -1, "Failed to initialize semaphore");
    
    for (int i = 0; i < MAX_OF; i++) { // init free vars
        of->vacant[i] = 1;
        of->queue[i] = NULL;
    }
    
    add_proc(of, box_id);

    return of;
}

int rem_proc(struct office * of, const unsigned int box_id) {
    if (check_err(box_id >= MAX_OF || of->vacant[box_id],
        "Attempted to remove invalid mailbox")) {
        return -1;
    }

    sem_wait(&of->sem[box_id]);
    sem_close(&of->sem[box_id]);
    of->sem[box_id];
    of->procs--;
    of->vacant[box_id] = 1;

    return 1;
}

int add_proc(struct office * of, const unsigned int box_id) {
    if (check_err(box_id >= MAX_OF || !of->vacant[box_id],
        "Attempted to add invalid mailbox")) {
        return -1;
    }

    sem_init(&of->sem[box_id], 0, 1);
    of->procs++;
    of->vacant[box_id] ^= of->vacant[box_id];

    return 1;
}

void write_proc(struct office * of, const char * msg) {
    of->procs = strlen(msg);

    for (size_t j = 0; j < of->procs; j++) {
        of->buff[j] = msg[j];
    }

    sem_post(&of->sem);
}

char * read_of(struct office * of) {
    char buff[MAX_OF];

    sem_wait(&of->sem);
    memcpy(buff, of->buff, of->procs);
    of->procs = 0;
    sem_post(&of->sem);

    return of->buff;
}

#endif