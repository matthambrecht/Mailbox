# Mailbox - A POSIX Shared Memory Multi-Directional Message Queue Library
[![Unit Tests](https://github.com/matthambrecht/Mailbox/actions/workflows/run-tests.yaml/badge.svg)](https://github.com/matthambrecht/Mailbox/actions/workflows/run-tests.yaml)

This is a C library to facilitate IPC (interprocess communication) by allowing
multi-directional message sharing. It is designed to mimic the organization of postal systems and made to be thread-safe using POSIX `semaphore.h`.

## What is IPC (interprocess communication)
"In computer science, interprocess communication (IPC) is the sharing of data between running processes in a computer system. Mechanisms for IPC may be provided by an operating system. Applications which use IPC are often categorized as clients and servers, where the client requests data and the server responds to client requests. Many applications are both clients and servers, as commonly seen in distributed computing." - [Wikipedia/Inter-process Communication](https://en.wikipedia.org/wiki/Inter-process_communication)

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

### Basic Functions
To setup a Mailbox we need a local "post office". To create a post office we use the office initializer.
```c
struct office * init_office(char * key);
```
The `key` is going to be the same amongst all programs and is how each process designates which memory store to share. We can think of this as our mailing hub where all mail is routed through. All processes that wish to use the shared memory will need to connect to the office prior to doing anything else.

The `box_id` you can think of as the unique post address for each program. It is how the programs know where to send to, and/or allows you to depict where they recieved a message from. To create a mailbox you will use the following function:
```c
int add_mailbox(struct office * of, const unsigned int box_id);
```
Processes are capable of sharing the same `box_id`, or even having multiple. It is up to the user to safely handle this to ensure messaging runs smoothly amongst processes, threads, etc.

To quickly get the `box_id` of the next available mailbox there is the provided function:
```c
int get_vacant(struct office * of);
```

To send a message we need the "post address" of the program we wish to send to, our "post office" which was initialized in the prior step, and the actual "mail".
```c
int send_mail(struct office * of, const unsigned int box_id, M * mail);
```

To check your mail you need your "post office" and "post address" of the current process. If the mailbox is empty you will recieve a `NULL` response, otherwise you will recieve a pointer to the message retrieved. This only retrieves one message at a time.
```c
M * check_mail(struct office * of, const unsigned int box_id);
```

### Cleanup Functions
To remove a mailbox it is suggested that you add a call to `remove_mailbox` when you are done with the mailbox in a process, or before said process dies. This will ensure that when all references are gone, the shared memory chunk will be destroyed, this function is defined as follows:
```c
int remove_mailbox(struct office * of, const unsigned int box_id);
```

If you're using this in a client-server fashion, or just need to manually destroy the entire shared memory, use the `destroy_office` function with the key you initialized the offices with. The function is defined as follows:
```c
void destroy_office(const char * key);
```

### Simple Server and Client
To wait for new mail for an unknown amount of time we can use:
```c
M * await_mail(struct office * of, const unsigned int box_id);
```

Having the ability to wait for mail without knowing when it will arrive allows us to create a server-client relationship. A simple server would look as follows:
```c
office * of = init_office("key");
add_mailbox(&of, 0);

while (1) {
    M * msg = await_mail(&of, 0);

    if (msg != NULL) {
        printf("%s\n", msg->msg_buff);
    }
}
```

While a client would be able to simply message the server at any point with:
```c
M * msg;
office * of = init_office("key");

send_mail(*of, 0, msg);
```

Because of how this library is setup you aren't limited to having one server and client, you could have processes acting as both to facilitate seamless mutli-directional communication between processes,

To see some samples of this library working in action, check `sample/`.

## Error Codes
| Code         | Int | Description                                                  |
|--------------|-----|--------------------------------------------------------------|
| MB_SUCCESS  | 0 | The intended task completed successfully.
| MB_EMPTY     | -10  | The mailbox the process attempted to retrieve from is empty. |
| MB_FULL      | -11  | The mailbox the process attempted to send to is full.        |
| MB_NOT_FOUND | -12  | The mailbox with the provided ID doesn't exist.              |
| MB_MEM_ERR   | -13  | An error occurred while setting up the Shared Memory.        |
| MB_BAD_ID    | -14  | The provided ID is out of the allowed range.                 |
| MB_EEXIST    | -15  | The mailbox the process attempted to create already exists.  |
| MB_OFFICE_FULL | -16 | We have reached the maximum number of allowed mailboxes |