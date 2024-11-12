/*
 * Client pour le distributeur de tickets
 * - Envoie une demande de réservation au serveur
 * - Reçoit la confirmation ou une alternative de réservation
 * - Affiche le statut de la réservation à l'utilisateur
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include "../Include/spectacles.h"
#include "../Include/ipc_utils.h"

static void afficher_menu(void) {
    static const char *menu = "\nMenu:\n"
                             "1. Réserver un billet\n"
                             "2. Annuler une réservation\n"
                             "3. Modifier une réservation\n"
                             "4. Consulter les disponibilités\n"
                             "5. Quitter\n";
    printf("%s", menu);
}

static void afficher_liste_spectacles(void) {
    static const char *spectacles = "\nSpectacles disponibles :\n"
                                  "------------------------\n"
                                  "Spectacle 0 : Concert de musique classique\n"
                                  "Spectacle 1 : Pièce de théâtre\n";
    printf("%s", spectacles);
}

inline int gerer_message(int msgid, void *msg, size_t size, long type, int envoi) {
    int ret = envoi ? 
        envoyer_message(msgid, msg, size) : 
        recevoir_message(msgid, msg, size, type);
    
    if (ret == -1) {
        perror(envoi ? "Erreur d'envoi" : "Erreur de réception");
        return 0;
    }
    return 1;
}

void reserver_billet(int msgid_demande, int msgid_reponse) {
    DemandeReservation demande;
    ReponseReservation reponse;

    afficher_liste_spectacles();

    printf("\nCatégories disponibles :\n");
    printf("------------------------\n");
    printf("0 - VIP       : 100€\n");
    printf("1 - Standard  : 50€\n");
    printf("2 - Économique: 25€\n");

    demande.user_id = getpid();
    printf("\nVotre ID utilisateur est : %d (généré automatiquement)\n", demande.user_id);

    do {
        printf("\nEntrez l'ID du spectacle (0 ou 1): ");
        scanf("%d", &demande.spectacle_id);
        if (demande.spectacle_id < 0 || demande.spectacle_id > 1) {
            printf("ID de spectacle invalide. Veuillez choisir 0 ou 1.\n");
        }
    } while (demande.spectacle_id < 0 || demande.spectacle_id > 1);

    do {
        printf("Entrez la catégorie (0: VIP, 1: Standard, 2: Economique): ");
        scanf("%d", &demande.categorie);
        if (demande.categorie < 0 || demande.categorie >= MAX_CATEGORIES) {
            printf("Catégorie invalide. Veuillez choisir entre 0 et 2.\n");
        }
    } while (demande.categorie < 0 || demande.categorie >= MAX_CATEGORIES);

    demande.type = 1; 

    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur lors de l'envoi de la demande de réservation");
        return;
    }

    if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
        perror("Erreur lors de la réception de la réponse de réservation");
        return;
    }

    if (reponse.success) {
        printf("\nRéservation réussie!\n");
        printf("Votre numéro de réservation : %d-%d-%d\n", demande.user_id, demande.spectacle_id, demande.categorie);
        printf("Gardez ce numéro pour toute modification ou annulation future.\n");
    } else {
        printf("\nRéservation échouée. ");
        if (reponse.categorie_suggeree != -1) {
            printf("Catégorie alternative suggérée: %d\n", reponse.categorie_suggeree);
        } else {
            printf("Aucune alternative disponible.\n");
        }
    }
}

void annuler_reservation(int msgid_demande, int msgid_reponse) {
    DemandeReservation demande;
    ReponseReservation reponse;
    ReservationInfo reservations[10];
    int nb_reservations = 0;
    int choix;

    printf("Entrez votre ID utilisateur: ");
    scanf("%d", &demande.user_id);
    demande.type = 5;  

    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur lors de l'envoi de la demande");
        return;
    }

    printf("\nVos réservations :\n");
    printf("------------------\n");

    while (1) {
        if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
            perror("Erreur lors de la réception des réservations");
            return;
        }

        if (reponse.spectacle_id == -1) break;  

        nb_reservations++;
        printf("%d. Spectacle %d (%s)\n", nb_reservations, reponse.spectacle_id,
               reponse.categorie == 0 ? "VIP" : (reponse.categorie == 1 ? "Standard" : "Economique"));
        
        reservations[nb_reservations-1].spectacle_id = reponse.spectacle_id;
        reservations[nb_reservations-1].categorie = reponse.categorie;
    }

    if (nb_reservations == 0) {
        printf("Vous n'avez aucune réservation.\n");
        return;
    }

    printf("\nQuelle réservation souhaitez-vous annuler ? (1-%d, 0 pour annuler) : ", nb_reservations);
    scanf("%d", &choix);

    if (choix > 0 && choix <= nb_reservations) {
        int confirmation;
        printf("Êtes-vous sûr de vouloir annuler cette réservation ? (1: Oui, 0: Non) : ");
        scanf("%d", &confirmation);

        if (confirmation == 1) {
            demande.type = 2;  
            demande.spectacle_id = reservations[choix-1].spectacle_id;
            demande.categorie = reservations[choix-1].categorie;

            if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
                perror("Erreur lors de l'envoi de la demande d'annulation");
                return;
            }

            if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
                perror("Erreur lors de la réception de la réponse d'annulation");
                return;
            }

            if (reponse.success) {
                printf("Annulation réussie!\n");
            } else {
                printf("Échec de l'annulation.\n");
            }
        } else {
            printf("Annulation abandonnée.\n");
        }
    } else if (choix != 0) {
        printf("Choix invalide.\n");
    }
}

void modifier_reservation(int msgid_demande, int msgid_reponse) {
    DemandeReservation demande;
    ReponseReservation reponse;
    ReservationInfo reservations[10];
    int nb_reservations = 0;
    int choix;

    printf("Entrez votre ID utilisateur: ");
    scanf("%d", &demande.user_id);
    demande.type = 5; 

    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur lors de l'envoi de la demande");
        return;
    }

    printf("\nVos réservations :\n");
    printf("------------------\n");

    while (1) {
        if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
            perror("Erreur lors de la réception des réservations");
            return;
        }

        if (reponse.spectacle_id == -1) break;  

        nb_reservations++;
        printf("%d. Spectacle %d (%s)\n", nb_reservations, reponse.spectacle_id,
               reponse.categorie == 0 ? "VIP" : (reponse.categorie == 1 ? "Standard" : "Economique"));
        
        reservations[nb_reservations-1].spectacle_id = reponse.spectacle_id;
        reservations[nb_reservations-1].categorie = reponse.categorie;
    }

    if (nb_reservations == 0) {
        printf("Vous n'avez aucune réservation à modifier.\n");
        return;
    }

    printf("\nQuelle réservation souhaitez-vous modifier ? (1-%d, 0 pour annuler) : ", nb_reservations);
    scanf("%d", &choix);

    if (choix > 0 && choix <= nb_reservations) {
        printf("\nNouvelle catégorie souhaitée (0: VIP, 1: Standard, 2: Economique): ");
        int nouvelle_categorie;
        scanf("%d", &nouvelle_categorie);

        if (nouvelle_categorie < 0 || nouvelle_categorie >= MAX_CATEGORIES) {
            printf("Catégorie invalide.\n");
            return;
        }

        demande.type = 3;  
        demande.spectacle_id = reservations[choix-1].spectacle_id;
        demande.categorie = reservations[choix-1].categorie;  
        demande.new_categorie = nouvelle_categorie;           

        // Envoyer la demande de modification
        if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
            perror("Erreur lors de l'envoi de la demande de modification");
            return;
        }

        // Recevoir la réponse de la modification
        if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
            perror("Erreur lors de la réception de la réponse de modification");
            return;
        }

        if (reponse.success) {
            printf("Modification réussie!\n");
        } else {
            printf("Modification échouée. ");
            if (reponse.categorie_suggeree != -1) {
                printf("Catégorie alternative suggérée: %d\n", reponse.categorie_suggeree);
            } else {
                printf("Aucune alternative disponible.\n");
            }
        }
    } else if (choix != 0) {
        printf("Choix invalide.\n");
    }
}

void consulter_disponibilites(int msgid_demande, int msgid_reponse) {
    DemandeReservation demande;
    ReponseReservation reponse;

    demande.user_id = getpid();
    demande.type = 4; 

    afficher_liste_spectacles();

    do {
        printf("\nEntrez l'ID du spectacle (0 ou 1): ");
        scanf("%d", &demande.spectacle_id);
        if (demande.spectacle_id < 0 || demande.spectacle_id > 1) {
            printf("ID de spectacle invalide. Veuillez choisir 0 ou 1.\n");
        }
    } while (demande.spectacle_id < 0 || demande.spectacle_id > 1);

    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur lors de l'envoi de la demande de consultation");
        return;
    }

    if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
        perror("Erreur lors de la réception de la réponse de consultation");
        return;
    }

    printf("\nDisponibilités pour le spectacle %d:\n", demande.spectacle_id);
    printf("------------------------\n");
    printf("VIP       : %d places\n", reponse.places_disponibles[0]);
    printf("Standard  : %d places\n", reponse.places_disponibles[1]);
    printf("Economique: %d places\n", reponse.places_disponibles[2]);
}

int main() {
    int msgid_demande = msgget(MSG_KEY_DEMANDE, 0666);
    int msgid_reponse = msgget(MSG_KEY_REPONSE, 0666);
    if (msgid_demande == -1 || msgid_reponse == -1) {
        perror("Erreur d'accès aux files de messages");
        exit(1);
    }

    int choix;
    do {
        afficher_menu();
        printf("Choisissez une option: ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                reserver_billet(msgid_demande, msgid_reponse);
                break;
            case 2:
                annuler_reservation(msgid_demande, msgid_reponse);
                break;
            case 3:
                modifier_reservation(msgid_demande, msgid_reponse);
                break;
            case 4:
                consulter_disponibilites(msgid_demande, msgid_reponse);
                break;
            case 5:
                printf("Au revoir!\n");
                break;
            default:
                printf("Option invalide, veuillez réessayer.\n");
        }
    } while (choix != 5);

    return 0;
}
