#ifndef SPECTACLES_H
#define SPECTACLES_H

#define MAX_CATEGORIES 3
#define MSG_KEY_DEMANDE 1234
#define MSG_KEY_REPONSE 5678

// Structures pour les spectacles et la réservation
typedef struct {
    int id;
    int places_disponibles[MAX_CATEGORIES];
} Spectacle;

typedef struct {
    long type;
    int user_id;
    int spectacle_id;
    int categorie;
} DemandeReservation;

typedef struct {
    long type;
    int success;
    int categorie_suggeree;
} ReponseReservation;

// Prototypes de fonctions ajoutées
void ajouter_spectacle(Spectacle *spectacles, int *nb_spectacles, int id, int places[]);
void annuler_reservation(Spectacle *spectacle, int categorie);
int trouver_alternative(Spectacle spectacle);
void afficher_spectacles(Spectacle *spectacles, int nb_spectacles);

#endif // SPECTACLES_H