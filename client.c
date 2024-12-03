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
#include <string.h>

static void afficher_menu(double solde) {
    static const char *menu = "\nMenu:\n"
                             "Votre solde: %.2f€\n"
                             "1. Réserver un billet\n"
                             "2. Annuler une réservation\n"
                             "3. Modifier une réservation\n"
                             "4. Consulter les disponibilités\n"
                             "5. Déconnexion\n";
    printf(menu, solde);
}

static void afficher_liste_spectacles(void) {
    static const char *spectacles = "\nSpectacles disponibles :\n"
                                  "------------------------\n"
                                  "Spectacle 0 : Concert de musique classique\n"
                                  "Spectacle 1 : Pièce de théâtre\n"
                                  "Spectacle 2 : Magie - Speculous rapidous\n"
                                  "Spectacle 3 : World League of Legends\n";

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

int simuler_paiement(int msgid_demande, int msgid_reponse, int user_id, int categorie) {
    printf("\nTraitement du paiement en cours");
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
    return 1;
}

void reserver_billet(int msgid_demande, int msgid_reponse, int user_id) {
    DemandeReservation demande;
    ReponseReservation reponse;

    afficher_liste_spectacles();

    printf("\nCatégories disponibles :\n");
    printf("------------------------\n");
    printf("0 - VIP       : 100€\n");
    printf("1 - Standard  : 50€\n");
    printf("2 - Économique: 25€\n");

    demande.user_id = user_id;
    printf("\nRéservation pour l'utilisateur ID: %d\n", user_id);

    do {
        printf("\nEntrez l'ID du spectacle (0 à 3): ");
        scanf("%d", &demande.spectacle_id);
        if (demande.spectacle_id < 0 || demande.spectacle_id > 3) {
            printf("ID de spectacle invalide. Veuillez choisir entre 0 et 3.\n");
        }
    } while (demande.spectacle_id < 0 || demande.spectacle_id > 3);

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
        double prix = 0.0;
        switch(demande.categorie) {
            case 0: prix = PRIX_VIP; break;
            case 1: prix = PRIX_STANDARD; break;
            case 2: prix = PRIX_ECO; break;
        }
        
        // Simulation du paiement 
        printf("\nTraitement du paiement en cours");
        for (int i = 0; i < 3; i++) {
            printf(".");
            fflush(stdout); 
            sleep(1);        
        }
        printf("\n");

        printf("\nRéservation réussie!\n");
        printf("Votre numéro de réservation : %d-%d-%d\n", 
               demande.user_id, demande.spectacle_id, demande.categorie);
        printf("Montant débité : %.2f€\n", prix);
        printf("Solde restant : %.2f€\n", reponse.solde_restant);
        printf("Gardez ce numéro pour toute modification ou annulation future.\n");
    } else {
        if (reponse.categorie_suggeree == -2) {
            printf("\nVous ne disposez pas d'assez d'argent pour effectuer cette réservation.\n");
            printf("Solde actuel : %.2f€\n", reponse.solde_restant);
        } else if (reponse.categorie_suggeree != -1) {
            char choix;
            printf("\nLa catégorie demandée n'est pas disponible.");
            printf("\nCatégorie alternative suggérée: %d (%s)", 
                   reponse.categorie_suggeree,
                   reponse.categorie_suggeree == 0 ? "VIP" : 
                   (reponse.categorie_suggeree == 1 ? "Standard" : "Économique"));
            
            printf("\nSouhaitez-vous réserver dans cette catégorie ? (o/n): ");
            scanf(" %c", &choix);
            
            if (choix == 'o' || choix == 'O') {
                // Réutiliser la même demande avec la nouvelle catégorie
                demande.categorie = reponse.categorie_suggeree;
                
                if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
                    perror("Erreur lors de l'envoi de la demande de réservation alternative");
                    return;
                }
                
                if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
                    perror("Erreur lors de la réception de la réponse de réservation alternative");
                    return;
                }
                
                if (reponse.success) {
                    double prix = 0.0;
                    switch(demande.categorie) {
                        case 0: prix = PRIX_VIP; break;
                        case 1: prix = PRIX_STANDARD; break;
                        case 2: prix = PRIX_ECO; break;
                    }
                    
                    printf("\nTraitement du paiement en cours");
                    for (int i = 0; i < 3; i++) {
                        printf(".");
                        fflush(stdout);
                        sleep(1);
                    }
                    printf("\n");
                    
                    printf("\nRéservation alternative réussie!\n");
                    printf("Votre numéro de réservation : %d-%d-%d\n", 
                           demande.user_id, demande.spectacle_id, demande.categorie);
                    printf("Montant débité : %.2f€\n", prix);
                    printf("Solde restant : %.2f€\n", reponse.solde_restant);
                    printf("Gardez ce numéro pour toute modification ou annulation future.\n");
                } else {
                    printf("\nLa réservation alternative a échoué. Veuillez réessayer plus tard.\n");
                }
            } else {
                printf("\nRéservation annulée.\n");
            }
        } else {
            printf("\nAucune alternative disponible. Vérifiez la disponibilité des places.\n");
        }
    }
}

void annuler_reservation(int msgid_demande, int msgid_reponse, int user_id) {
    DemandeReservation demande;
    ReponseReservation reponse;
    ReservationInfo reservations[10];
    int nb_reservations = 0;
    int choix;

    demande.user_id = user_id;
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

            printf("\nAnnulation en cours");
            for (int i = 0; i < 3; i++) {
                printf(".");
                fflush(stdout);
                sleep(1);
            }
            printf("\n");

            if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
                perror("Erreur lors de la réception de la réponse d'annulation");
                return;
            }

            if (reponse.success) {
                printf("Annulation réussie!\n");
                printf("Montant remboursé : %.2f€\n", reponse.montant_rembourse);
                printf("Nouveau solde : %.2f€\n", reponse.solde_restant);
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

void modifier_reservation(int msgid_demande, int msgid_reponse, int user_id) {
    DemandeReservation demande;
    ReponseReservation reponse;
    ReservationInfo reservations[10];
    int nb_reservations = 0;
    int choix;

    demande.user_id = user_id;
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

        if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
            perror("Erreur lors de l'envoi de la demande de modification");
            return;
        }

        printf("\nModification en cours");
        for (int i = 0; i < 3; i++) {
            printf(".");
            fflush(stdout);
            sleep(1);
        }
        printf("\n");

        if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
            perror("Erreur lors de la réception de la réponse de modification");
            return;
        }

        if (reponse.success) {
            if (reponse.difference_prix > 0) {
                printf("\nTraitement du paiement supplémentaire en cours");
                for (int i = 0; i < 3; i++) {
                    printf(".");
                    fflush(stdout);
                    sleep(1);
                }
                printf("\n");
            }
            printf("Modification réussie!\n");
            if (reponse.difference_prix > 0) {
                printf("Supplément débité : %.2f€\n", reponse.difference_prix);
            } else if (reponse.difference_prix < 0) {
                printf("Montant remboursé : %.2f€\n", -reponse.difference_prix);
            }
            printf("Nouveau solde : %.2f€\n", reponse.solde_restant);
        } else {
            printf("Modification échouée. ");
            if (reponse.categorie_suggeree != -1) {
                printf("Catégorie alternative suggérée: %d\n", reponse.categorie_suggeree);
            } else {
                printf("Vérifiez votre solde ou la disponibilité des places.\n");
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
        printf("\nEntrez l'ID du spectacle (0 à 3): ");
        scanf("%d", &demande.spectacle_id);
        if (demande.spectacle_id < 0 || demande.spectacle_id > 3) {
            printf("ID de spectacle invalide. Veuillez choisir entre 0 ou 3.\n");
        }
    } while (demande.spectacle_id < 0 || demande.spectacle_id > 3);

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

static void afficher_menu_connexion(void) {
    static const char *menu = "\nMenu de Connexion:\n"
                             "1. Se connecter\n"
                             "2. Créer un compte\n"
                             "3. Quitter\n";
    printf("%s", menu);
}

int connecter_utilisateur(int msgid_demande, int msgid_reponse) {
    DemandeReservation demande;
    ReponseReservation reponse;
    
    memset(&demande, 0, sizeof(DemandeReservation));
    
    printf("Username: ");
    scanf("%s", demande.username);
    getchar(); 
    
    const char *pass = getpass("Password: "); 
    strncpy(demande.password, pass, MAX_PASSWORD - 1);
    
    demande.type = 6;
    demande.user_id = getpid();
    
    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur lors de l'envoi de la demande de connexion");
        return -1;
    }
    
    if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
        perror("Erreur lors de la réception de la réponse");
        return -1;
    }
    
    if (reponse.success) {
        printf("Connexion réussie! Bienvenue %s\n", demande.username);
        return reponse.user_id;
    } else {
        printf("Échec de la connexion. Vérifiez vos identifiants.\n");
        return -1;
    }
}

int creer_compte(int msgid_demande, int msgid_reponse) {
    DemandeReservation demande;
    ReponseReservation reponse;
    
    memset(&demande, 0, sizeof(DemandeReservation));
    
    printf("Choisissez un username: ");
    scanf("%s", demande.username);
    getchar(); 
    
    const char *pass = getpass("Choisissez un password: "); 
    strncpy(demande.password, pass, MAX_PASSWORD - 1);
    
    do {
        printf("Entrez votre solde initial (minimum 25€): ");
        scanf("%lf", &demande.solde_initial);
        if (demande.solde_initial < 25.0) {
            printf("Le solde initial doit être d'au moins 25€ (prix minimum d'un billet)\n");
        }
    } while (demande.solde_initial < 25.0);
    
    demande.type = 7;
    demande.user_id = getpid();
    
    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur lors de l'envoi de la demande de création");
        return -1;
    }
    
    if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), demande.user_id) == -1) {
        perror("Erreur lors de la réception de la réponse");
        return -1;
    }
    
    if (reponse.success) {
        printf("Compte créé avec succès! Votre ID est %d\n", reponse.user_id);
        reponse.solde_restant = demande.solde_initial;
        return reponse.user_id;
    } else {
        printf("Échec de la création du compte. Username déjà pris.\n");
        return -1;
    }
}

void gerer_menu_principal(int msgid_demande, int msgid_reponse, int user_id) {
    int choix;
    double solde_actuel = 0.0;

    // Demande du solde initial
    DemandeReservation demande;
    ReponseReservation reponse;
    
    memset(&demande, 0, sizeof(DemandeReservation));
    demande.type = 8;  // Type pour obtenir le solde
    demande.user_id = user_id;
    
    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) != -1) {
        if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), user_id) != -1) {
            solde_actuel = reponse.solde_restant;
        }
    }
    
    do {
        afficher_menu(solde_actuel);
        printf("Votre choix: ");
        scanf("%d", &choix);
        
        switch(choix) {
            case 1:
                reserver_billet(msgid_demande, msgid_reponse, user_id);
                break;
            case 2:
                annuler_reservation(msgid_demande, msgid_reponse, user_id);
                break;
            case 3:
                modifier_reservation(msgid_demande, msgid_reponse, user_id);
                break;
            case 4:
                consulter_disponibilites(msgid_demande, msgid_reponse);
                break;
            case 5:
                printf("Déconnexion...\n");
                break;
            default:
                printf("Option invalide\n");
        }

        // Mettre à jour le solde après chaque opération
        memset(&demande, 0, sizeof(DemandeReservation));
        demande.type = 8;
        demande.user_id = user_id;
        
        if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) != -1) {
            if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), user_id) != -1) {
                solde_actuel = reponse.solde_restant;
            }
        }
    } while (choix != 5);
}

int main() {
    int msgid_demande = msgget(MSG_KEY_DEMANDE, 0666);
    int msgid_reponse = msgget(MSG_KEY_REPONSE, 0666);
    if (msgid_demande == -1 || msgid_reponse == -1) {
        perror("Erreur d'accès aux files de messages");
        exit(1);
    }

    int choix;
    int user_id = -1;
    
    do {
        afficher_menu_connexion();
        printf("Votre choix: ");
        scanf("%d", &choix);
        
        switch(choix) {
            case 1:
                user_id = connecter_utilisateur(msgid_demande, msgid_reponse);
                if (user_id != -1) {
                    gerer_menu_principal(msgid_demande, msgid_reponse, user_id);
                }
                break;
            case 2:
                user_id = creer_compte(msgid_demande, msgid_reponse);
                if (user_id != -1) {
                    gerer_menu_principal(msgid_demande, msgid_reponse, user_id);
                }
                break;
            case 3:
                printf("Au revoir!\n");
                break;
            default:
                printf("Option invalide\n");
        }
    } while (choix != 3);

    return 0;
}
