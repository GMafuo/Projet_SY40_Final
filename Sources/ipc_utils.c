// ipc_utils.c
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Include/ipc_utils.h"

int create_msg_queue(int key) {
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Erreur lors de la création de la file de messages");
        exit(1);
    }
    return msgid;
}

void delete_msg_queue(int msgid) {
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Erreur lors de la suppression de la file de messages");
    }
}
