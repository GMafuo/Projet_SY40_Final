#include "../Include/spectacles.h"
#include <stdio.h>

// Ajouter un nouveau spectacle
void ajouter_spectacle(Spectacle *spectacles, int *nb_spectacles, int id, int places[]) {
    spectacles[*nb_spectacles].id = id;
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        spectacles[*nb_spectacles].places_disponibles[i] = places[i];
    }
    (*nb_spectacles)++;
    printf("Spectacle %d ajouté avec %d places par catégorie\n", id, places[0]);
}

// Renommé pour éviter le conflit
void annuler_reservation_spectacle(Spectacle *spectacle, int categorie, int user_id) {
    if (categorie >= MAX_CATEGORIES || categorie < 0) {
        printf("Catégorie invalide\n");
        return;
    }
    
    for (int i = 0; i < spectacle->nb_reservations; i++) {
        if (spectacle->reservations[i].user_id == user_id && 
            spectacle->reservations[i].categorie == categorie &&
            spectacle->reservations[i].active == RESERVATION_ACTIVE) {
            
            spectacle->reservations[i].active = RESERVATION_ANNULEE;
            spectacle->places_disponibles[categorie]++;
            
            printf("Annulation effectuée avec succès\n");
            return;
        }
    }
    
    printf("Aucune réservation active trouvée pour cet utilisateur\n");
}

// Ajouter une réservation
int ajouter_reservation(Spectacle *spectacle, int categorie, int user_id) {
    if (categorie >= MAX_CATEGORIES || categorie < 0) {
        printf("Catégorie invalide\n");
        return 0;
    }
    
    if (spectacle->places_disponibles[categorie] <= 0) {
        printf("Plus de places disponibles dans cette catégorie\n");
        return 0;
    }
    
    for (int i = 0; i < spectacle->nb_reservations; i++) {
        if (spectacle->reservations[i].user_id == user_id && 
            spectacle->reservations[i].active == RESERVATION_ACTIVE) {
            printf("L'utilisateur a déjà une réservation active\n");
            return 0;
        }
    }
    
    if (spectacle->nb_reservations >= MAX_RESERVATIONS) {
        printf("Nombre maximum de réservations atteint\n");
        return 0;
    }
    
    int idx = spectacle->nb_reservations;
    spectacle->reservations[idx].user_id = user_id;
    spectacle->reservations[idx].spectacle_id = spectacle->id;
    spectacle->reservations[idx].categorie = categorie;
    spectacle->reservations[idx].active = RESERVATION_ACTIVE;
    spectacle->nb_reservations++;
    spectacle->places_disponibles[categorie]--;
    
    printf("Réservation effectuée avec succès\n");
    return 1;
}

// Fonction pour trouver une catégorie alternative disponible
int trouver_alternative(Spectacle spectacles[], int nb_spectacles, int spectacle_id) {
    for (int i = 0; i < nb_spectacles; i++) {
        if (i != spectacle_id) { // Vérifier d'autres spectacles
            for (int j = 0; j < MAX_CATEGORIES; j++) {
                if (spectacles[i].places_disponibles[j] > 0) {
                    return j; // Retourne l'index de la première catégorie disponible
                }
            }
        }
    }
    return -1; // Aucune place disponible dans les catégories
}

void modifier_reservation_spectacle(Spectacle *spectacles, int nb_spectacles, int spectacle_id, int old_categorie, int new_categorie, int user_id) {
    if (spectacle_id >= nb_spectacles || new_categorie >= MAX_CATEGORIES || new_categorie < 0) {
        printf("Paramètres invalides pour la modification de réservation\n");
        return;
    }
    
    Spectacle *spectacle = &spectacles[spectacle_id];
    
    // Vérifier si la nouvelle catégorie a des places disponibles
    if (spectacle->places_disponibles[new_categorie] <= 0) {
        printf("Plus de places disponibles dans la nouvelle catégorie\n");
        return;
    }
    
    // Chercher la réservation existante
    for (int i = 0; i < spectacle->nb_reservations; i++) {
        if (spectacle->reservations[i].user_id == user_id && 
            spectacle->reservations[i].categorie == old_categorie &&
            spectacle->reservations[i].active == RESERVATION_ACTIVE) {
            
            // Modifier la réservation
            spectacle->reservations[i].categorie = new_categorie;
            spectacle->places_disponibles[old_categorie]++;
            spectacle->places_disponibles[new_categorie]--;
            
            printf("Modification de réservation effectuée avec succès\n");
            return;
        }
    }
    
    printf("Aucune réservation active trouvée pour cet utilisateur dans la catégorie spécifiée\n");
}

// Afficher les spectacles et les places disponibles
void afficher_spectacles(Spectacle *spectacles, int nb_spectacles) {
    for (int i = 0; i < nb_spectacles; i++) {
        printf("Spectacle ID: %d - Places disponibles: VIP %d, Standard %d, Economique %d\n",
               spectacles[i].id,
               spectacles[i].places_disponibles[0],
               spectacles[i].places_disponibles[1],
               spectacles[i].places_disponibles[2]);
    }
}

// Supprimer un spectacle
void supprimer_spectacle(Spectacle *spectacles, int *nb_spectacles, int id) {
    for (int i = 0; i < *nb_spectacles; i++) {
        if (spectacles[i].id == id) {
            for (int j = i; j < *nb_spectacles - 1; j++) {
                spectacles[j] = spectacles[j + 1];
            }
            (*nb_spectacles)--;
            printf("Spectacle %d supprimé\n", id);
            return;
        }
    }
    printf("Spectacle %d non trouvé\n", id);
}

// Modifier un spectacle
void modifier_spectacle(Spectacle *spectacles, int nb_spectacles, int id, int new_places[]) {
    for (int i = 0; i < nb_spectacles; i++) {
        if (spectacles[i].id == id) {
            for (int j = 0; j < MAX_CATEGORIES; j++) {
                spectacles[i].places_disponibles[j] = new_places[j];
            }
            printf("Spectacle %d modifié\n", id);
            return;
        }
    }
    printf("Spectacle %d non trouvé\n", id);
}
