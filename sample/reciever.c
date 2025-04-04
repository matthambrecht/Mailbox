#include <stdio.h>

#include "../mailbox.h"

int main() {
    struct office *of = init_office("sample_mb");
    add_mailbox(of, 1);

    for (int i = 0; i < MAX_Q_LEN; i++) {
        M * o = await_mail(of, 1);

        if (o != NULL) {
            printf("%s\n", o->msg_buff);
        }
    }

    destroy_office("sample_mb");
    
    return 0;
}