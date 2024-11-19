/*
 * Serveur de réservation pour le distributeur de tickets
 * - Reçoit les demandes de réservation des clients
 * - Vérifie les places disponibles et confirme ou propose une alternative
 * - Gestion des spectacles et des réservations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>
#include "../Include/spectacles.h"
#include "../Include/ipc_utils.h"

int msgid_demande, msgid_reponse;
User users[MAX_USERS];
int nb_users = 0;
sem_t *sem_spectacles;
sem_t *sem_users;

// Fonction de nettoyage pour supprimer les files de messages à la fin
void nettoyer_ressources() {
    delete_msg_queue(msgid_demande);
    delete_msg_queue(msgid_reponse);
    
    // Fermer et supprimer les sémaphores
    sem_close(sem_spectacles);
    sem_close(sem_users);
    sem_unlink(SEM_SPECTACLES);
    sem_unlink(SEM_USERS);
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

        // Initialise la réponse
        reponse.type = demande.user_id;
        reponse.success = 0;

        switch(demande.type) {
            case 1: // Réservation
                {
                    memset(&reponse, 0, sizeof(ReponseReservation));
                    reponse.type = demande.user_id;
                    
                    if (demande.spectacle_id < nb_spectacles) {
                        Spectacle *spectacle = &spectacles[demande.spectacle_id];
                        double solde = obtenir_solde_utilisateur(demande.user_id);
                        double prix = obtenir_prix_categorie(demande.categorie);
                        
                        // Vérifie d'abord si la catégorie demandée est disponible
                        if (spectacle->places_disponibles[demande.categorie] > 0) {
                            if (solde >= prix) {
                                if (ajouter_reservation(spectacle, demande.categorie, demande.user_id, sem_spectacles)) {
                                    mettre_a_jour_solde(demande.user_id, prix);
                                    reponse.success = 1;
                                    reponse.solde_restant = obtenir_solde_utilisateur(demande.user_id);
                                }
                            } else {
                                reponse.success = 0;
                                reponse.categorie_suggeree = -2;
                                reponse.solde_restant = solde;
                                printf("Paiement refusé - solde insuffisant (%.2f€)\n", solde);
                            }
                        } else {
                            // Chercher une alternative
                            int categorie_alternative = trouver_alternative(spectacle, demande.categorie);
                            if (categorie_alternative != -1) {
                                double prix_alternative = obtenir_prix_categorie(categorie_alternative);
                                if (solde >= prix_alternative) {
                                    reponse.success = 0;
                                    reponse.categorie_suggeree = categorie_alternative;
                                    reponse.solde_restant = solde;
                                    printf("Proposition d'alternative : catégorie %d\n", categorie_alternative);
                                } else {
                                    reponse.success = 0;
                                    reponse.categorie_suggeree = -2;
                                    reponse.solde_restant = solde;
                                    printf("Solde insuffisant pour toute alternative (%.2f€)\n", solde);
                                }
                            } else {
                                reponse.success = 0;
                                reponse.categorie_suggeree = -1;
                                reponse.solde_restant = solde;
                                printf("Aucune alternative disponible\n");
                            }
                        }
                    }
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse de réservation échoué");
                    }
                }
                break;

            case 2: // Annulation
                if (demande.spectacle_id < nb_spectacles) {
                    double prix_remboursement = obtenir_prix_categorie(demande.categorie);
                    
                    annuler_reservation_spectacle(&spectacles[demande.spectacle_id], 
                                                demande.categorie,
                                                demande.user_id,
                                                sem_spectacles);
                    
                    mettre_a_jour_solde(demande.user_id, -prix_remboursement);
                    
                    reponse.success = 1;
                    reponse.montant_rembourse = prix_remboursement;
                    reponse.solde_restant = obtenir_solde_utilisateur(demande.user_id);
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse d'annulation échoué");
                    }
                }
                break;

            case 3: // Modification
                if (demande.spectacle_id < nb_spectacles) {
                    double ancien_prix = obtenir_prix_categorie(demande.categorie);
                    double nouveau_prix = obtenir_prix_categorie(demande.new_categorie);
                    double difference = nouveau_prix - ancien_prix;
                    
                    if (difference > 0 && obtenir_solde_utilisateur(demande.user_id) < difference) {
                        reponse.success = 0;
                    } else {
                        modifier_reservation_spectacle(spectacles, 
                                                    nb_spectacles, 
                                                    demande.spectacle_id, 
                                                    demande.categorie,      
                                                    demande.new_categorie,  
                                                    demande.user_id,
                                                    sem_spectacles);
                        
                        if (difference != 0) {
                            mettre_a_jour_solde(demande.user_id, difference);
                        }
                        
                        reponse.success = 1;
                        reponse.difference_prix = difference;
                        reponse.solde_restant = obtenir_solde_utilisateur(demande.user_id);
                    }
                    
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

            case 6: // Connexion
                {
                    int user_id = verifier_credentials(demande.username, demande.password);
                    reponse.type = demande.user_id;
                    reponse.success = (user_id != -1);
                    reponse.user_id = user_id;
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse de connexion échoué");
                    }
                }
                break;

            case 7: // Création de compte
                {
                    memset(&reponse, 0, sizeof(ReponseReservation));
                    int user_id = creer_utilisateur(demande.username, demande.password, demande.solde_initial);
                    reponse.type = demande.user_id;  
                    reponse.success = (user_id != -1);
                    reponse.user_id = user_id;
                    
                    if (envoyer_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long)) == -1) {
                        perror("Erreur : Envoi de la réponse de création de compte échoué");
                    } else {
                        printf("Compte créé avec succès pour l'utilisateur %s (ID: %d) avec solde initial de %.2f€\n", 
                               demande.username, user_id, demande.solde_initial);
                    }
                }
                break;
        }
    }
}

int verifier_credentials(const char *username, const char *password) {
    for (int i = 0; i < nb_users; i++) {
        if (strcmp(users[i].username, username) == 0 && 
            strcmp(users[i].password, password) == 0 && 
            users[i].active) {
            return users[i].id;
        }
    }
    return -1;
}

int creer_utilisateur(const char *username, const char *password, double solde_initial) {
    sem_wait(sem_users);
    
    // Vérifie si username existe déjà
    for (int i = 0; i < nb_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            sem_post(sem_users);
            return -1;
        }
    }
    
    if (nb_users >= MAX_USERS) {
        sem_post(sem_users);
        return -1;
    }
    
    strcpy(users[nb_users].username, username);
    strcpy(users[nb_users].password, password);
    users[nb_users].id = nb_users + 1;
    users[nb_users].active = 1;
    users[nb_users].solde = solde_initial;  
    
    printf("Nouveau compte créé: %s (ID: %d) avec solde initial de %.2f€\n",
           username, users[nb_users].id, users[nb_users].solde);
    
    int new_user_id = users[nb_users].id;
    nb_users++;
    
    sem_post(sem_users);
    return new_user_id;
}

double obtenir_prix_categorie(int categorie) {
    switch(categorie) {
        case 0: return PRIX_VIP;
        case 1: return PRIX_STANDARD;
        case 2: return PRIX_ECO;
        default: return 0.0;
    }
}

double obtenir_solde_utilisateur(int user_id) {
    sem_wait(sem_users);
    double solde = -1.0;
    
    for (int i = 0; i < nb_users; i++) {
        if (users[i].id == user_id && users[i].active) {
            solde = users[i].solde;
            break;
        }
    }
    
    sem_post(sem_users);
    return solde;
}

void mettre_a_jour_solde(int user_id, double montant) {
    sem_wait(sem_users);
    
    for (int i = 0; i < nb_users; i++) {
        if (users[i].id == user_id && users[i].active) {
            users[i].solde -= montant;
            printf("Solde mis à jour pour l'utilisateur %d: %.2f€\n", 
                   user_id, users[i].solde);
            break;
        }
    }
    
    sem_post(sem_users);
}

int effectuer_paiement(int user_id, int categorie) {
    double prix = obtenir_prix_categorie(categorie);
    double solde = obtenir_solde_utilisateur(user_id);
    
    if (solde < prix) {
        return 0;  
    }
    
    // Temps de traitement du paiement
    sleep(2);
    
    mettre_a_jour_solde(user_id, prix);
    return 1;  
}

int main() {
    msgid_demande = create_msg_queue(MSG_KEY_DEMANDE);
    msgid_reponse = create_msg_queue(MSG_KEY_REPONSE);

    atexit(nettoyer_ressources);

    // Initialiser les sémaphores
    sem_spectacles = sem_open(SEM_SPECTACLES, O_CREAT, 0666, 1);
    sem_users = sem_open(SEM_USERS, O_CREAT, 0666, 1);
    
    if (sem_spectacles == SEM_FAILED || sem_users == SEM_FAILED) {
        perror("Erreur création sémaphores");
        exit(1);
    }

    // Initialise les spectacles
    Spectacle spectacles[2] = {
        {0, {5, 3, 2}}, // Spectacle 0 avec 5 places VIP, 3 Standard, 2 Économique
        {1, {2, 4, 1}}  // Spectacle 1 avec 2 places VIP, 4 Standard, 1 Économique
    };

    printf("Serveur prêt à recevoir des demandes...\n");
    traiter_demandes_reservation(spectacles, 2);

    return 0;
}
