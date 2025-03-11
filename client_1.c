#include <stdio.h>
#include "mailbox.h"

int main() {
    struct Mailbox *mb = init_mailbox("ooga2");

    write_mb(mb, "Hey there");

    return 0;
}