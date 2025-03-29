/*
mailbox.h - A Unix Shared Memory Mailing System for IPC

Author: Matthew Hambrecht (https://github.com/matthambrecht)
Version: v1.2.0
Documentation: https://github.com/matthambrecht/Mailbox/blob/main/README.md
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

// Error codes
#define MB_SUCCESS 0
#define MB_EMPTY -10
#define MB_FULL -11
#define MB_NOT_FOUND -12
#define MB_MEM_ERR -13
#define MB_BAD_ID -14
#define MB_EEXIST -15
#define MB_OFFICE_FULL -16

// Config consts
#define MAX_MB 24 // Max number of processes capable of having a mailbox
#define MAX_Q_LEN 64 // Max amount of mail per mailbox

typedef struct {
    char msg_buff[256];
    size_t msg_len;
} M;

// Our distribution center types
struct mailbox {
    M objs[MAX_Q_LEN];
    size_t len;
    unsigned int front;
    unsigned int back;
};

struct office {
    const char * key;
    unsigned int vacant[MAX_MB];
    sem_t mutex[MAX_MB];
    sem_t notif[MAX_MB];
    char buff[MAX_MB];
    struct mailbox mb[MAX_MB];
    size_t procs;
};

// error handlers
void check_perr(int fail);

// queue fn declarations
struct mailbox mb_init();
int mb_insert(struct mailbox * mb, M * mail);
M * mb_pop(struct mailbox * mb);
int mb_full(struct mailbox * mb);
int mb_empty(struct mailbox * mb);

// office fn declarations
struct office * connect(const char * key);
struct office * init_office(const char * key);
void destroy_office(const char * key);
int remove_mailbox(struct office * of, const unsigned int box_id);
int add_mailbox(struct office * of, const unsigned int box_id);
int send_mail(struct office * of, const unsigned int box_id, M * mail);
M * check_mail(struct office * of, const unsigned int box_id);
M * await_mail(struct office * of, const unsigned int box_id);


// Error handling stuff
void check_perr(int fail) { // for errors something else caught
    if (fail) {
        exit(EIO);
    }
}


// Mailbox queue stuff
struct mailbox mb_init() {
    struct mailbox q;
    
    q.len = 0;
    q.front = 0;
    q.back = 0;

    return q;
};


int mb_insert(struct mailbox * mb, M * mail) {
    if (mb_full(mb)) {
        return MB_FULL;
    }

    mb->objs[mb->back] = *mail;
    mb->back = (mb->back + 1) % MAX_Q_LEN;
    mb->len++;

    return MB_SUCCESS;
}


M * mb_pop(struct mailbox * mb) {
    if (mb_empty(mb)) {
        return NULL;
    }

    M * out = &mb->objs[mb->front];
    mb->front = (mb->front + 1) % MAX_Q_LEN;
    mb->len--;

    return out;
}


int mb_empty(struct mailbox * mb) {
    return !mb->len;
}


int mb_full(struct mailbox * mb) {
    return mb->len == MAX_Q_LEN;
}


// Office state stuff
struct office * connect(const char * key) { // connect to existing
    int fd;
    struct office * of;

    fd = shm_open(
        key,
        O_RDWR,
        0
    );

    check_perr(fd == -1);
    of = (struct office *) mmap(
        NULL,
        sizeof(*of),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    check_perr(fd == -1);

    return of;
}


struct office * init_office(const char * key) {
    int fd;
    struct office * of;

    fd = shm_open(
        key,
        O_CREAT | O_EXCL | O_RDWR,
        0600
    );

    if (fd == -1) {
        if (errno == EEXIST) { // office exists
            return connect(key);
        }

        check_perr(-1);
    }

    check_perr(ftruncate(fd, sizeof(struct office)) == -1);

    of = (struct office *) mmap(
        NULL,
        sizeof(*of),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    check_perr(of == MAP_FAILED);
    
    of->key = key;

    for (int i = 0; i < MAX_MB; i++) { // init free vars
        sem_init(&of->notif[i], 1, 0);
        sem_init(&of->mutex[i], 1, 1);
        of->vacant[i] = 1;
    }

    return of;
}


void destroy_office(const char * key) {
    shm_unlink(key);
}

int get_vacant(struct office * of) {
    for (size_t i = 0; i < MAX_MB; i++) {
        if (of->vacant[i]) {
            return i;
        }
    }

    return MB_OFFICE_FULL;
}

int remove_mailbox(struct office * of, const unsigned int box_id) {
    if (box_id >= MAX_MB) {
        return MB_BAD_ID;
    }

    if (of->vacant[box_id]) {
        return MB_NOT_FOUND;
    }

    sem_wait(&of->mutex[box_id]);
    sem_close(&of->mutex[box_id]);
    of->procs--;
    of->vacant[box_id] = 1;

    if (!of->procs) {
        destroy_office(of->key);
    }

    return MB_SUCCESS;
}


int add_mailbox(struct office * of, const unsigned int box_id) {
    if (box_id >= MAX_MB) {
        return MB_BAD_ID;
    }

    if (!of->vacant[box_id]) {
        return MB_EEXIST;
    }

    struct mailbox mb = mb_init();
    sem_init(&of->mutex[box_id], 0, 1);
    of->mb[box_id] = mb;
    of->procs++;
    of->vacant[box_id] = 0;

    return MB_SUCCESS;
}


// Message passing stuff
int send_mail(struct office * of, const unsigned int box_id, M * mail) {
    if (box_id >= MAX_MB) {
        return MB_BAD_ID;
    }
    
    if (of->vacant[box_id]) {
        return MB_NOT_FOUND;
    }

    sem_wait(&of->mutex[box_id]);
    int rv = mb_insert(&of->mb[box_id], mail);
    sem_post(&of->mutex[box_id]);
    sem_post(&of->notif[box_id]);

    return rv;
}


M * check_mail(struct office * of, const unsigned int box_id) {
    if (box_id >= MAX_MB) {
        return NULL;
    }
    
    if (of->vacant[box_id]) {
        return NULL;
    }

    sem_wait(&of->mutex[box_id]);
    M * out = mb_pop(&of->mb[box_id]);
    sem_post(&of->mutex[box_id]);

    if (!mb_empty(&of->mb[box_id])) { // Keep semaphore state
        sem_post(&of->notif[box_id]);
    }

    return out;
}


M * await_mail(struct office * of, const unsigned int box_id) {
    if (box_id >= MAX_MB) {
        return NULL;
    }
    
    if (of->vacant[box_id]) {
        return NULL;
    }

    sem_wait(&of->notif[box_id]);

    return check_mail(of, box_id);
}
#endif