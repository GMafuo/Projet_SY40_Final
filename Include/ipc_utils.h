#ifndef IPC_UTILS_H
#define IPC_UTILS_H

#include <stddef.h>

int create_msg_queue(int key);
void delete_msg_queue(int msgid);
int envoyer_message(int msgid, void *msg, size_t size);
int recevoir_message(int msgid, void *msg, size_t size, long type);

#endif // IPC_UTILS_H
