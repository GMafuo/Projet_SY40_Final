#include "../Include/ipc_utils.h"
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

// Fichier contenant les utilitaires pour la communication inter-processus (IPC)

// Crée une nouvelle file de messages avec les droits d'accès 0666
int create_msg_queue(int key) {
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Erreur lors de la création de la file de messages");
        exit(1);
    }
    return msgid;
}

// Supprime une file de messages existante
void delete_msg_queue(int msgid) {
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Erreur lors de la suppression de la file de messages");
    }
}

// Envoie un message dans une file de messages
// Fonction générique qui peut envoyer n'importe quel type de message
int envoyer_message(int msgid, void *msg, size_t size) {
    return msgsnd(msgid, msg, size, 0);
}

// Reçoit un message d'une file de messages
// Fonction générique qui peut recevoir n'importe quel type de message
int recevoir_message(int msgid, void *msg, size_t size, long type) {
    return msgrcv(msgid, msg, size, type, 0);
}
