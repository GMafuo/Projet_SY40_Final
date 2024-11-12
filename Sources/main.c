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
            perror("Erreur : Impossible de recevoir la demande");
            continue;
        }

        // Initialiser la réponse
        reponse.type = demande.user_id;
        reponse.success = 0;

        switch(demande.type) {
            case 1: // Réservation
                if (demande.spectacle_id < nb_spectacles) {
                    reponse.success = ajouter_reservation(&spectacles[demande.spectacle_id], 
                                                        demande.categorie, 
                                                        demande.user_id);
                    if (!reponse.success) {
                        reponse.categorie_suggeree = trouver_alternative(spectacles, 
                                                                       nb_spectacles, 
                                                                       demande.spectacle_id);
                    } else {
                        reponse.categorie_suggeree = -1;
                    }
                    
                    if (envoyer_message(msgid_reponse, &reponse, 
                                       sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse échoué");
                    }
                }
                break;

            case 2: // Annulation
                if (demande.spectacle_id < nb_spectacles) {
                    annuler_reservation_spectacle(&spectacles[demande.spectacle_id], 
                                                demande.categorie,
                                                demande.user_id);
                    reponse.success = 1;
                    printf("Annulation confirmée pour l'utilisateur %d\n", demande.user_id);
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse d'annulation échoué");
                    }
                }
                break;

            case 3: // Modification
                if (demande.spectacle_id < nb_spectacles) {
                    modifier_reservation_spectacle(spectacles, 
                                                 nb_spectacles, 
                                                 demande.spectacle_id, 
                                                 demande.categorie,      
                                                 demande.new_categorie,  
                                                 demande.user_id);
                    reponse.success = 1;
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse de modification échoué");
                    }
                }
                break;

            case 4: // Consultation des disponibilités
                if (demande.spectacle_id < nb_spectacles) {
                    reponse.type = demande.user_id;  
                    
                    for (int i = 0; i < MAX_CATEGORIES; i++) {
                        reponse.places_disponibles[i] = spectacles[demande.spectacle_id].places_disponibles[i];
                    }
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse de consultation échoué");
                    }
                }
                break;

            case 5: // Consultation des réservations
                {
                    int found = 0;
                    for (int i = 0; i < nb_spectacles; i++) {
                        for (int j = 0; j < spectacles[i].nb_reservations; j++) {
                            if (spectacles[i].reservations[j].user_id == demande.user_id && 
                                spectacles[i].reservations[j].active == 1) {
                                found = 1;
                                reponse.spectacle_id = spectacles[i].id;
                                reponse.categorie = spectacles[i].reservations[j].categorie;
                                
                                if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                                    perror("Erreur : Envoi de la réponse échoué");
                                }
                            }
                        }
                    }
                    
                    if (!found) {
                        printf("Aucune réservation active trouvée pour l'utilisateur %d\n", demande.user_id);
                    }
                    
                    reponse.spectacle_id = -1;
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi du message de fin échoué");
                    }
                }
                break;
        }
    }
}

int main() {
    msgid_demande = create_msg_queue(MSG_KEY_DEMANDE);
    msgid_reponse = create_msg_queue(MSG_KEY_REPONSE);

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
