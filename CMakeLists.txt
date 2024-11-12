cmake_minimum_required(VERSION 3.10)
project(distributeur_tickets C)

# Inclure le répertoire pour les fichiers d'en-tête
include_directories(Include)

# Cible pour le serveur principal
add_executable(serveur
        Sources/main.c
        Sources/ipc_utils.c
        Sources/spectacles.c
)

# Cible pour le client
add_executable(client
        client.c
        Sources/ipc_utils.c
        Sources/spectacles.c
)
