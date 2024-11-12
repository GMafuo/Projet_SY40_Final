#include "../Include/ipc_utils.h"
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

// Création d'une file de messages
int create_msg_queue(int key) {
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Erreur lors de la création de la file de messages");
        exit(1);
    }
    return msgid;
}

// Suppression d'une file de messages
void delete_msg_queue(int msgid) {
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Erreur lors de la suppression de la file de messages");
    }
}

// Envoi d'un message générique
int envoyer_message(int msgid, void *msg, size_t size) {
    return msgsnd(msgid, msg, size, 0);
}

// Réception d'un message générique
int recevoir_message(int msgid, void *msg, size_t size, long type) {
    return msgrcv(msgid, msg, size, type, 0);
}
