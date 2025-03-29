#include <stdio.h>

#include "../mailbox.h"

int main() {
    struct office *of = init_office("sample_mb");
    add_mailbox(of, 3);

    for (int i = 0; i < MAX_Q_LEN; i++) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Hello world, this is message %d", i);

        M mail = {
            {0},
            strlen(msg)
        }; 

        memcpy(mail.msg_buff, msg, strlen(msg));
        send_mail(of, 1, &mail);
    }

    return 0;
}