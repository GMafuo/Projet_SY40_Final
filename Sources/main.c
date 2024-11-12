/*
 * Serveur de gestion des réservations de spectacles
 * Gère les demandes des clients via des files de messages IPC
 * Maintient l'état des spectacles et des réservations
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include "../Include/spectacles.h"
#include "../Include/ipc_utils.h"

int msgid_demande, msgid_reponse;

// Nettoie les ressources IPC à la fermeture du serveur
void nettoyer_ressources() {
    delete_msg_queue(msgid_demande);
    delete_msg_queue(msgid_reponse);
}

// Boucle principale de traitement des demandes clients
void traiter_demandes_reservation(Spectacle spectacles[], int nb_spectacles) {
    DemandeReservation demande;
    ReponseReservation reponse;

    while (1) {
        // Attend et traite les demandes selon leur type
        // Type 1: Nouvelle réservation
        // Type 2: Annulation
        // Type 3: Modification
        // Type 4: Consultation
        // Type 5: Liste des réservations d'un client
        
        // ... code de traitement ...
    }
}

int main() {
    // Initialisation des files de messages
    msgid_demande = create_msg_queue(MSG_KEY_DEMANDE);
    msgid_reponse = create_msg_queue(MSG_KEY_REPONSE);

    // Nettoyage automatique à la fermeture
    atexit(nettoyer_ressources);

    // Configuration initiale des spectacles
    Spectacle spectacles[2] = {
        {0, {5, 3, 2}}, // Spectacle 0: 5 VIP, 3 Standard, 2 Eco
        {1, {2, 4, 1}}  // Spectacle 1: 2 VIP, 4 Standard, 1 Eco
    };

    // Démarrage du serveur
    printf("Serveur prêt à recevoir des demandes...\n");
    traiter_demandes_reservation(spectacles, 2);

    return 0;
}
