//
// Created by simon on 12/11/24.
//
// main.c
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Include/ipc_utils.h"
#include "../Include/spectacles.h"

void traiter_demandes_reservation(int msgid_demande, int msgid_reponse, Spectacle spectacles[], int nb_spectacles) {
    DemandeReservation demande;
    ReponseReservation reponse;

    while (1) {
        // Lire la demande dans la file de messages
        if (msgrcv(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long), 0, 0) == -1) {
            perror("Erreur lors de la réception de la demande de réservation");
            exit(1);
        }

        printf("Demande reçue de l'utilisateur %d pour le spectacle %d, catégorie %d\n", demande.user_id, demande.spectacle_id, demande.categorie);

        // Vérifier la disponibilité de la catégorie demandée
        if (demande.spectacle_id < nb_spectacles && spectacles[demande.spectacle_id].places_disponibles[demande.categorie] > 0) {
            // Réservation réussie
            spectacles[demande.spectacle_id].places_disponibles[demande.categorie]--;
            reponse.success = 1;
            reponse.categorie_suggeree = -1;
            printf("Réservation confirmée pour l'utilisateur %d\n", demande.user_id);
        } else {
            // Réservation échouée, proposer une alternative
            reponse.success = 0;
            reponse.categorie_suggeree = trouver_alternative(spectacles[demande.spectacle_id]);
            printf("Aucune place disponible dans la catégorie demandée. Alternative proposée : catégorie %d\n", reponse.categorie_suggeree);
        }

        // Envoyer la réponse à l'utilisateur
        reponse.type = demande.user_id;
        if (msgsnd(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), 0) == -1) {
            perror("Erreur lors de l'envoi de la réponse de réservation");
            exit(1);
        }
    }
}

int main() {
    int msgid_demande = create_msg_queue(MSG_KEY_DEMANDE);
    int msgid_reponse = create_msg_queue(MSG_KEY_REPONSE);

    Spectacle spectacles[2] = {
        {0, {5, 3, 2}}, // Spectacle 0 avec 5 places VIP, 3 Standard, 2 Économique
        {1, {2, 4, 1}}  // Spectacle 1 avec 2 places VIP, 4 Standard, 1 Économique
    };

    traiter_demandes_reservation(msgid_demande, msgid_reponse, spectacles, 2);

    delete_msg_queue(msgid_demande);
    delete_msg_queue(msgid_reponse);

    return 0;
}

