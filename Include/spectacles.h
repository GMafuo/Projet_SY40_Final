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

// Déclaration de la fonction trouver_alternative
int trouver_alternative(Spectacle spectacle);

void envoyer_demande_reservation(int msgid_demande, int user_id, int spectacle_id, int categorie);

#endif // SPECTACLES_H
