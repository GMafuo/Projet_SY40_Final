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
void annuler_reservation_spectacle(Spectacle *spectacle, int categorie, int user_id, sem_t *sem) {
    sem_wait(sem);
    
    if (categorie >= MAX_CATEGORIES || categorie < 0) {
        printf("Catégorie invalide\n");
        sem_post(sem);
        return;
    }
    
    for (int i = 0; i < spectacle->nb_reservations; i++) {
        if (spectacle->reservations[i].user_id == user_id && 
            spectacle->reservations[i].categorie == categorie &&
            spectacle->reservations[i].active == RESERVATION_ACTIVE) {
            
            spectacle->reservations[i].active = RESERVATION_ANNULEE;
            spectacle->places_disponibles[categorie]++;
            printf("Annulation effectuée avec succès\n");
            sem_post(sem);
            return;
        }
    }
    
    printf("Aucune réservation active trouvée pour cet utilisateur\n");
    sem_post(sem);
}

// Ajouter une réservation
int ajouter_reservation(Spectacle *spectacle, int categorie, int user_id, sem_t *sem) {
    int success = 0;
    
    sem_wait(sem);  // Verrouiller l'accès
    
    if (categorie >= MAX_CATEGORIES || categorie < 0) {
        printf("Catégorie invalide\n");
        sem_post(sem);  // Déverrouiller avant de sortir
        return 0;
    }
    
    if (spectacle->places_disponibles[categorie] <= 0) {
        printf("Plus de places disponibles dans cette catégorie\n");
        sem_post(sem);
        return 0;
    }
    
    if (spectacle->nb_reservations < MAX_RESERVATIONS) {
        int idx = spectacle->nb_reservations;
        spectacle->reservations[idx].user_id = user_id;
        spectacle->reservations[idx].spectacle_id = spectacle->id;
        spectacle->reservations[idx].categorie = categorie;
        spectacle->reservations[idx].active = RESERVATION_ACTIVE;
        spectacle->nb_reservations++;
        spectacle->places_disponibles[categorie]--;
        success = 1;
        printf("Réservation effectuée avec succès\n");
    }
    
    sem_post(sem);  
    return success;
}

int trouver_alternative(const Spectacle *spectacle, int categorie_demandee) {
    // D'abord essayer la catégorie supérieure
    if (categorie_demandee > 0) {
        for (int cat = categorie_demandee - 1; cat >= 0; cat--) {
            if (spectacle->places_disponibles[cat] > 0) {
                return cat;
            }
        }
    }
    
    // Ensuite essayer la catégorie inférieure
    if (categorie_demandee < MAX_CATEGORIES - 1) {
        for (int cat = categorie_demandee + 1; cat < MAX_CATEGORIES; cat++) {
            if (spectacle->places_disponibles[cat] > 0) {
                return cat;
            }
        }
    }
    
    return -1; 
}

void modifier_reservation_spectacle(Spectacle *spectacles, int nb_spectacles, 
                                  int spectacle_id, int old_categorie, 
                                  int new_categorie, int user_id, sem_t *sem) {
    sem_wait(sem);
    
    if (spectacle_id >= nb_spectacles || new_categorie >= MAX_CATEGORIES || new_categorie < 0) {
        printf("Paramètres invalides pour la modification de réservation\n");
        sem_post(sem);
        return;
    }
    
    Spectacle *spectacle = &spectacles[spectacle_id];
    
    if (spectacle->places_disponibles[new_categorie] <= 0) {
        printf("Plus de places disponibles dans la nouvelle catégorie\n");
        sem_post(sem);
        return;
    }
    
    for (int i = 0; i < spectacle->nb_reservations; i++) {
        if (spectacle->reservations[i].user_id == user_id && 
            spectacle->reservations[i].categorie == old_categorie &&
            spectacle->reservations[i].active == RESERVATION_ACTIVE) {
            
            spectacle->reservations[i].categorie = new_categorie;
            spectacle->places_disponibles[old_categorie]++;
            spectacle->places_disponibles[new_categorie]--;
            
            printf("Modification de réservation effectuée avec succès\n");
            sem_post(sem);
            return;
        }
    }
    
    printf("Aucune réservation active trouvée pour cet utilisateur\n");
    sem_post(sem);
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
