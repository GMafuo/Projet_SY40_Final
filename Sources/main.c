/*
 * Serveur de réservation pour le distributeur de tickets
 * - Reçoit les demandes de réservation des clients
 * - Vérifie les places disponibles et confirme ou propose une alternative
 * - Gestion des spectacles et des réservations
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include "../Include/spectacles.h"
#include "../Include/ipc_utils.h"

int msgid_demande, msgid_reponse;

// Fonction de nettoyage pour supprimer les files de messages à la fin
void nettoyer_ressources() {
    delete_msg_queue(msgid_demande);
    delete_msg_queue(msgid_reponse);
}

// Fonction principale de traitement des demandes de réservation
void traiter_demandes_reservation(Spectacle spectacles[], int nb_spectacles) {
    DemandeReservation demande;
    ReponseReservation reponse;

    while (1) {
        // Lire la demande dans la file de messages
        if (recevoir_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long), 0) == -1) {
            perror("Erreur : Impossible de recevoir la demande de réservation");
            continue;
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
        if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
            perror("Erreur : Envoi de la réponse de réservation échoué");
        }
    }
}

int main() {
    msgid_demande = create_msg_queue(MSG_KEY_DEMANDE);
    msgid_reponse = create_msg_queue(MSG_KEY_REPONSE);

    // Enregistrer la fonction de nettoyage pour la fin du programme
    atexit(nettoyer_ressources);

    // Initialiser les spectacles
    Spectacle spectacles[2] = {
        {0, {5, 3, 2}}, // Spectacle 0 avec 5 places VIP, 3 Standard, 2 Économique
        {1, {2, 4, 1}}  // Spectacle 1 avec 2 places VIP, 4 Standard, 1 Économique
    };

    printf("Serveur prêt à recevoir des demandes...\n");
    traiter_demandes_reservation(spectacles, 2);

    return 0;
}
