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

// Annuler une réservation dans une catégorie spécifique
void annuler_reservation(Spectacle *spectacle, int categorie) {
    if (categorie < MAX_CATEGORIES && spectacle->places_disponibles[categorie] < MAX_CATEGORIES) {
        spectacle->places_disponibles[categorie]++;
        printf("Annulation effectuée dans la catégorie %d\n", categorie);
    } else {
        printf("Erreur lors de l'annulation\n");
    }
}

// Fonction pour trouver une catégorie alternative disponible
// Retourne l'index de la première catégorie avec des places disponibles ou -1 si aucune place n'est disponible
int trouver_alternative(Spectacle spectacle) {
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        if (spectacle.places_disponibles[i] > 0) {
            return i; // Retourne l'index de la première catégorie disponible
        }
    }
    return -1; // Aucune place disponible dans les catégories
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
