# Système de Réservation de Spectacles

Ce projet implémente un simulateur de distributeur automatique de tickets pour des spectacles, permettant une gestion en temps réel des réservations via des mécanismes IPC.

## Fonctionnalités

- Réservation de billets pour différents spectacles
- 3 catégories de places : VIP, Standard, Économique
- Annulation et modification de réservations
- Consultation des disponibilités
- Gestion des réservations par utilisateur
- Suggestion automatique d'alternatives en cas d'indisponibilité

## Prérequis

- GCC ou autre compilateur C
- CMake (version 3.10 minimum)
- Système d'exploitation Linux/Unix (pour les IPC)

## Installation

1. Cloner le dépôt :
```bash
git clone git@github.com:GMafuo/Projet_SY40_Final.git
cd Projet_SY40_Final
```

2. Compiler le projet :
```bash
cd build
cmake ..
make
```

## Utilisation

1. Démarrer le serveur :
```bash
./serveur
```

2. Dans un autre terminal, démarrer le client :
```bash
./client
```

3. Suivre les instructions du menu interactif pour :
   - Réserver un billet
   - Annuler une réservation
   - Modifier une réservation
   - Consulter les disponibilités

## Structure du Projet

```
Projet_SY40_Final/
├── Include/
│   ├── spectacles.h    # Définitions des structures et constantes
│   └── ipc_utils.h     # Utilitaires IPC
├── Sources/
│   ├── main.c          # Serveur principal
│   ├── spectacles.c    # Gestion des spectacles
│   └── ipc_utils.c     # Implémentation IPC
└── client.c            # Programme client
```

## Architecture

- Communication client-serveur via files de messages IPC
- Gestion des spectacles en mémoire côté serveur
- Interface utilisateur interactive côté client
- Gestion des erreurs et des cas limites

## Auteurs

- Simon Nicod
- Simon Nguyen
- Mathéo Girard
