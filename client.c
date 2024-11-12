//
// Created by simon on 12/11/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include "Include/spectacles.h"

int main() {
    int msgid_demande = msgget(MSG_KEY_DEMANDE, 0666);
    int msgid_reponse = msgget(MSG_KEY_REPONSE, 0666);
    if (msgid_demande == -1 || msgid_reponse == -1) {
        perror("Erreur d'accès aux files de messages");
        exit(1);
    }

    int user_id = getpid();  // Utiliser le PID pour l'identifier
    int spectacle_id = 0;    // Par exemple, demander une place pour le spectacle 0
    int categorie = 1;       // Par exemple, demander une place dans la catégorie Standard

    // Envoyer la demande de réservation
    envoyer_demande_reservation(msgid_demande, user_id, spectacle_id, categorie);

    // Lire la réponse du serveur
    ReponseReservation reponse;
    if (msgrcv(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), user_id, 0) == -1) {
        perror("Erreur lors de la réception de la réponse de réservation");
        exit(1);
    }

    if (reponse.success) {
        printf("Réservation confirmée pour le spectacle %d, catégorie %d\n", spectacle_id, categorie);
    } else if (reponse.categorie_suggeree != -1) {
        printf("Catégorie demandée pleine. Alternative proposée : catégorie %d\n", reponse.categorie_suggeree);
    } else {
        printf("Pas de place disponible pour le spectacle %d\n", spectacle_id);
    }

    return 0;
}
