#include <stdio.h>
#include <stdlib.h>

#include "../mailbox.h"

/* 
Test Notes: 
    ID Purposes:
        - 0: Normal mailbox
        - 1: Destination mailbox
        - 2: Mailbox that will never be created at any point
        - 3: Existent but always empty
*/

// Utility declarations
void pass(const char * test) {printf("\033[0;32m[Pass]\033[0m %s\n", test);};
int fail(const char * test) {printf("\033[0;31m[Fail]\033[0m %s\n", test);return EXIT_FAILURE;};
void c_assert(const char * test, const int status);
int test_send(struct office * of, const int dest);

// Test declarations
void init_normal_tests();
void send_normal_tests();
void send_edge_tests();
void send_error_tests();
void check_normal_tests();
void check_edge_tests();
void check_error_tests();
void remove_normal_tests();
void remove_edge_tests();
void remove_error_tests();
void destroy_normal_tests();
void destroy_edge_tests();

void c_assert(const char * test, const int status) {
    if (!status) {
        fail(test);
        exit(EXIT_FAILURE);
    }
}

int test_send(struct office * of, const int dest) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Hello world, this is message");

    M mail = {
        {0},
        strlen(msg)
    }; 

    return send_mail(of, dest, &mail);
}

void init_normal_tests() {
    struct office * of1 = init_office("test_of", 0);
    struct office * of2 = init_office("test_of", 1);

    pass(__func__);
}

void send_normal_tests() {
    struct office * of1 = init_office("test_of", 0);
    struct office * of2 = init_office("test_of", 1);

    // Send somewhere else
    c_assert(__func__, test_send(of1, 1) == MB_SUCCESS);

    pass(__func__);
}

void send_edge_tests() {
    struct office * of1 = init_office("test_of", 0);

    // Send self
    c_assert(__func__, test_send(of1, 0) == MB_SUCCESS);


    for (int i = 0; i < MAX_Q_LEN; i++) { // Fill up mailbox
        test_send(of1, 1);
    }

    // Send to full mailbox
    c_assert(__func__, test_send(of1, 1) == MB_FULL);

    pass(__func__);
}

void send_error_tests() {
    struct office * of1 = init_office("test_of", 0);
    struct office * of2 = init_office("test_of", 1);

    c_assert(__func__, test_send(of1, -1) == MB_BAD_ID); // OOB ID
    c_assert(__func__, test_send(of1, 2) == MB_NOT_FOUND); // Nonexistent ID

    pass(__func__);
}

void check_normal_tests() {
    struct office * of1 = init_office("test_of", 0);
    struct office * of2 = init_office("test_of", 1);

    // Check mailbox with atleast something
    c_assert(__func__, check_mail(of1, 1) != NULL);

    for (int i = 0; i < MAX_Q_LEN; i++) { // Empty mailbox
        check_mail(of1, 1);
    }

    c_assert(__func__, check_mail(of1, 1) == NULL);

    pass(__func__);
}

void check_edge_tests() {
    struct office * of1 = init_office("test_of", 3);

    c_assert(__func__, check_mail(of1, 3) == NULL);

    pass(__func__);
}

void check_error_tests() {
    struct office * of1 = init_office("test_of", 0);
    struct office * of2 = init_office("test_of", 1);

    c_assert(__func__, check_mail(of1, -1) == NULL); // OOB ID
    c_assert(__func__, check_mail(of1, 2) == NULL); // Nonexistent ID

    pass(__func__);
}

void remove_normal_tests() {
    struct office * of1 = init_office("test_of", 1);

    // Remove existent mailbox
    c_assert(__func__, remove_mailbox(of1, 1) == MB_SUCCESS);

    pass(__func__);
}

void remove_edge_tests() {
    struct office * of1 = init_office("test_of", 3);

    // Double delete test
    c_assert(__func__, remove_mailbox(of1, 3) == MB_SUCCESS);
    c_assert(__func__, remove_mailbox(of1, 3) == MB_NOT_FOUND);

    pass(__func__);
}

void remove_error_tests() {
    struct office * of1 = init_office("test_of", 0);

    c_assert(__func__, remove_mailbox(of1, -1) == MB_BAD_ID); // OOB ID
    c_assert(__func__, remove_mailbox(of1, 2) == MB_NOT_FOUND); // Nonexistent ID

    pass(__func__);
}

void destroy_normal_tests() {
    struct office * of1 = init_office("test_of", 0);

    destroy_office(of1->key);

    pass(__func__);
}

void destroy_edge_tests() {
    struct office * of1 = init_office("test_of", 0);

    destroy_office("test_of");
    destroy_office("test_of");

    pass(__func__);
}

int main() {
    // Init tests
    init_normal_tests();

    // Send tests
    send_normal_tests();
    send_edge_tests();
    send_error_tests();

    // Check tests
    check_normal_tests();
    check_edge_tests();
    check_error_tests();

    // Remove tests
    remove_normal_tests();
    remove_edge_tests();
    remove_error_tests();

    // Destroy Tests
    destroy_normal_tests();
    destroy_edge_tests();

    return 0;
}