#include <stdio.h>

#include "../mailbox.h"

int main() {
    struct office *of = init_office("sample_mb", 1);

    for (int i = 0; i < MAX_Q_LEN; i++) {
        M * o = check_mail(of, 1);

        if (o != NULL) {
            printf("%s\n", o->msg_buff);
        }
    }
    
    return 0;
}