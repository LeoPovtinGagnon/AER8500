#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int start_client(const std::string& server_ip, int server_port) {
    // Création du socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    // Activer SO_REUSEADDR pour permettre la réutilisation immédiate du port
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cerr << "Erreur lors de l'activation de SO_REUSEADDR." << std::endl;
        close(server_socket);
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    // Paramétrage de l'adresse du serveur
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr)); // Initialiser à 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Adresse IP invalide." << std::endl;
        close(server_socket);
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    // Connexion au serveur
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connexion au serveur échouée." << std::endl;
        close(server_socket);
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    std::cout << "Connecté au serveur." << std::endl;

    return server_socket;  // Retourne le server_socket si tout est ok
}