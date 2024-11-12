#ifndef SPECTACLES_H
#define SPECTACLES_H

// Constantes de configuration
#define MAX_CATEGORIES 3        // Nombre de catégories de places (VIP, Standard, Eco)
#define MSG_KEY_DEMANDE 1234   // Clé IPC pour la file des demandes client->serveur
#define MSG_KEY_REPONSE 5678   // Clé IPC pour la file des réponses serveur->client
#define MAX_RESERVATIONS 100   // Nombre maximum de réservations par spectacle
#define RESERVATION_ACTIVE 1    // État d'une réservation valide
#define RESERVATION_ANNULEE 0   // État d'une réservation annulée

// Structure représentant une réservation individuelle
typedef struct {
    int user_id;        // Identifiant unique du client
    int spectacle_id;   // Identifiant du spectacle
    int categorie;      // Catégorie de place (0: VIP, 1: Standard, 2: Eco)
    int active;         // État de la réservation
} Reservation;

// Structure principale d'un spectacle
typedef struct {
    int id;                                     // Identifiant unique du spectacle
    int places_disponibles[MAX_CATEGORIES];     // Nombre de places par catégorie
    Reservation reservations[MAX_RESERVATIONS]; // Tableau des réservations
    int nb_reservations;                        // Nombre total de réservations
} Spectacle;

// Structure pour les messages de demande client->serveur
typedef struct {
    long type;          // Type de message IPC (1: réservation, 2: annulation, etc.)
    int user_id;        // ID du client
    int spectacle_id;   // ID du spectacle concerné
    int categorie;      // Catégorie demandée
    int new_categorie;  // Nouvelle catégorie (pour modifications)
} DemandeReservation;

typedef struct {
    long type;
    int success;
    int categorie_suggeree;
    int spectacle_id;
    int categorie;
    int places_disponibles[MAX_CATEGORIES];
} ReponseReservation;

typedef struct {
    int spectacle_id;
    int categorie;
} ReservationInfo;

// Prototypes de fonctions
void ajouter_spectacle(Spectacle *spectacles, int *nb_spectacles, int id, int places[]);
void annuler_reservation_spectacle(Spectacle *spectacle, int categorie, int user_id);
int trouver_alternative(Spectacle spectacles[], int nb_spectacles, int spectacle_id);
void afficher_spectacles(Spectacle *spectacles, int nb_spectacles);
int ajouter_reservation(Spectacle *spectacle, int categorie, int user_id);
void modifier_reservation_spectacle(Spectacle *spectacles, int nb_spectacles, int spectacle_id, int old_categorie, int new_categorie, int user_id);

#endif // SPECTACLES_H

