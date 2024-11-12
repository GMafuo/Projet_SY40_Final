/*
 * Client pour le distributeur de tickets
 * - Envoie une demande de réservation au serveur
 * - Reçoit la confirmation ou une alternative de réservation
 * - Affiche le statut de la réservation à l'utilisateur
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>         // Inclure unistd.h pour getpid()
#include "../Include/spectacles.h"
#include "../Include/ipc_utils.h"

int main() {
    int msgid_demande = msgget(MSG_KEY_DEMANDE, 0666);
    int msgid_reponse = msgget(MSG_KEY_REPONSE, 0666);
    if (msgid_demande == -1 || msgid_reponse == -1) {
        perror("Erreur d'accès aux files de messages");
        exit(1);
    }

    int user_id = getpid();  // Utiliser getpid() pour obtenir l'ID de processus unique
    int spectacle_id = 0;    // Par exemple, demander une place pour le spectacle 0
    int categorie = 1;       // Par exemple, demander une place dans la catégorie Standard

    // Créer et envoyer la demande de réservation
    DemandeReservation demande;
    demande.type = 1;
    demande.user_id = user_id;
    demande.spectacle_id = spectacle_id;
    demande.categorie = categorie;

    if (envoyer_message(msgid_demande, &demande, sizeof(DemandeReservation) - sizeof(long)) == -1) {
        perror("Erreur : Envoi de la demande de réservation échoué");
        exit(1);
    }
    printf("Demande de réservation envoyée pour le spectacle %d, catégorie %d\n", spectacle_id, categorie);

    // Lire la réponse du serveur
    ReponseReservation reponse;
    if (recevoir_message(msgid_reponse, &reponse, sizeof(ReponseReservation) - sizeof(long), user_id) == -1) {
        perror("Erreur lors de la réception de la réponse de réservation");
        exit(1);
    }

    // Afficher le résultat de la réservation
    if (reponse.success) {
        printf("Réservation confirmée pour le spectacle %d, catégorie %d\n", spectacle_id, categorie);
    } else if (reponse.categorie_suggeree != -1) {
        printf("Catégorie demandée pleine. Alternative proposée : catégorie %d\n", reponse.categorie_suggeree);
    } else {
        printf("Pas de place disponible pour le spectacle %d\n", spectacle_id);
    }

    return 0;
}
