#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Include/spectacles.h"

void envoyer_demande_reservation(int msgid_demande, int user_id, int spectacle_id, int categorie) {
    DemandeReservation demande;
    demande.type = 1; // Type de message, peut être arbitraire
    demande.user_id = user_id;
    demande.spectacle_id = spectacle_id;
    demande.categorie = categorie;

    if (msgsnd(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la demande de réservation");
        exit(1);
    }
    printf("Demande de réservation envoyée pour l'utilisateur %d, spectacle %d, catégorie %d\n", user_id, spectacle_id, categorie);
}

int trouver_alternative(Spectacle spectacle) {
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        if (spectacle.places_disponibles[i] > 0) {
            return i; // Retourne la première catégorie disponible
        }
    }
    return -1; // Aucune place disponible dans les catégories
}

