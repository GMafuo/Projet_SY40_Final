#ifndef IPC_UTILS_H
#define IPC_UTILS_H

#include <stddef.h>

// Utilitaires de gestion des files de messages IPC

// Crée une file de messages avec la clé spécifiée
int create_msg_queue(int key);

// Supprime une file de messages existante
void delete_msg_queue(int msgid);

// Envoie un message dans la file
int envoyer_message(int msgid, void *msg, size_t size);

// Reçoit un message de la file
int recevoir_message(int msgid, void *msg, size_t size, long type);

#endif // IPC_UTILS_H
