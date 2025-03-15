# Mailbox - A POSIX Shared Memory Multi-Directional Message Queue
This is simple C a library to facilitate IPC (interprocess communication) allowing
multi-directional message sharing, similar to that of a standard mailing system.

## Setup
Design the object you would like to share between processes. By default it is:
```c
typedef struct {
    char msg_buff[256];
    size_t msg_len;
} M; 
```

You are also able to configure some basic parameters:
```c
#define MAX_MB 24 // Max number of processes capable of having a mailbox
#define MAX_Q_LEN 64 // Max amount of mail per mailbox
```

All of the aforementioned modifications should be made to `mailbox.h`.

## Usage
To include the library add it to your source directory and add the header to your file with `#include "mailbox.h"`

To setup a Mailbox we need a local "post office". To create a post office we use the office initializer.
```c
struct office * init_office(char * key, const unsigned int box_id);
```
The `key` is going to be the same amongst all programs and is how each process designates which memory store to share. The `box_id` you can think of as the unique post address for each program. It is how the programs know where to send to, and where they recieved a message from.

To send a message we need the "post address" of the program we wish to send to, a "post office" which was initialized in the prior step, and the actual "mail".
```c
int send_mail(struct office * of, const unsigned int box_id, M * mail);
```

To check your mail you need your "post office" and "post address" of the current process. If the mailbox is empty you will recieve a `NULL` response, otherwise you will recieve a pointer to the message retrieved.
```c
M * check_mail(struct office * of, const unsigned int box_id);
```

To see some samples of how this works in action, check `sample/`.

## Error Codes
| Code         | Int | Description                                                  |
|--------------|-----|--------------------------------------------------------------|
| MB_SUCCESS  | 0 | The intended task completed successfully.
| MB_EMPTY     | 10  | The mailbox the process attempted to retrieve from is empty. |
| MB_FULL      | 11  | The mailbox the process attempted to send to is full.        |
| MB_NOT_FOUND | 12  | The mailbox with the provided ID doesn't exist.              |
| MB_MEM_ERR   | 13  | An error occurred while setting up the Shared Memory.        |
| MB_BAD_ID    | 14  | The provided ID is out of the allowed range.                 |
| MB_EEXIST    | 15  | The mailbox the process attempted to create already exists.  |