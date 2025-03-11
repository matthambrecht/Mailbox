#include <stdio.h>
#include "mailbox.h"

int main() {
    struct Mailbox *mb = init_mailbox("ooga2");
    char * out;
    
    out = read_mb(mb);
    printf("%s", out);

    return 0;
}