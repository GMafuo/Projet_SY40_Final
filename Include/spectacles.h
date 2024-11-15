#ifndef SPECTACLES_H
#define SPECTACLES_H

#include <semaphore.h>
#include <fcntl.h>

// Constantes de configuration
#define MAX_CATEGORIES 3        // Nombre de catégories de places (VIP, Standard, Eco)
#define MSG_KEY_DEMANDE 1234   // Clé IPC pour la file des demandes client->serveur
#define MSG_KEY_REPONSE 5678   // Clé IPC pour la file des réponses serveur->client
#define MAX_RESERVATIONS 100   // Nombre maximum de réservations par spectacle
#define RESERVATION_ACTIVE 1    // État d'une réservation valide
#define RESERVATION_ANNULEE 0   // État d'une réservation annulée
#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 50

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
    sem_t *mutex;  // Mutex pour chaque spectacle
} Spectacle;

// Structure pour les messages de demande client->serveur
typedef struct {
    long type;          
    int user_id;        
    int spectacle_id;   
    int categorie;      
    int new_categorie;
    char username[MAX_USERNAME];  
    char password[MAX_PASSWORD]; 
} DemandeReservation;

// Structure pour les messages de réponse serveur->client
typedef struct {
    long type;
    int success;
    int categorie_suggeree;
    int spectacle_id;
    int categorie;
    int places_disponibles[MAX_CATEGORIES];
    int user_id;
    double montant_rembourse;    // Pour les annulations
    double difference_prix;      // Pour les modifications
    double solde_restant;       // Pour afficher le solde après opération
} ReponseReservation;

typedef struct {
    int spectacle_id;
    int categorie;
} ReservationInfo;

// Structure pour les utilisateurs
typedef struct {
    int id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int active;  // 1 si compte actif, 0 sinon
    double solde;  // Solde du compte en euros
} User;

// Definition sémaphore
#define SEM_SPECTACLES "/sem_spectacles"
#define SEM_USERS "/sem_users"

// Prototypes de fonctions
void ajouter_spectacle(Spectacle *spectacles, int *nb_spectacles, int id, int places[]);
void annuler_reservation_spectacle(Spectacle *spectacle, int categorie, int user_id, sem_t *sem);
int trouver_alternative(Spectacle spectacles[], int nb_spectacles, int spectacle_id);
void afficher_spectacles(Spectacle *spectacles, int nb_spectacles);
int ajouter_reservation(Spectacle *spectacle, int categorie, int user_id, sem_t *sem);
void modifier_reservation_spectacle(Spectacle *spectacles, int nb_spectacles, int spectacle_id, int old_categorie, int new_categorie, int user_id, sem_t *sem);
int verifier_credentials(const char *username, const char *password);
int creer_utilisateur(const char *username, const char *password);
int effectuer_paiement(int user_id, int categorie);
double obtenir_prix_categorie(int categorie);
double obtenir_solde_utilisateur(int user_id);
void mettre_a_jour_solde(int user_id, double montant);

#define PRIX_VIP 100.0
#define PRIX_STANDARD 50.0
#define PRIX_ECO 25.0
#define SOLDE_INITIAL 500.0  

#endif // SPECTACLES_H

