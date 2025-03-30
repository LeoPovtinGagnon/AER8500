
#include "server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib> // Pour rand()
#include <ctime>   // Pour time()

int start_server(const std::string& server_ip, int server_port) {
    // Création du socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return -1;
    }

    // Paramétrage de l'adresse du serveur
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr)); // Initialiser à 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Lier le socket à l'adresse
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erreur lors du bind." << std::endl;
        close(server_socket);
        return -1;
    }

    // Écouter les connexions entrantes
    if (listen(server_socket, 3) < 0) {
        std::cerr << "Erreur lors de l'écoute des connexions." << std::endl;
        close(server_socket);
        return -1;
    }

    std::cout << "Serveur en attente de connexion..." << std::endl;

    // Attendre la connexion du client
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket < 0) {
        std::cerr << "Erreur lors de l'acceptation de la connexion." << std::endl;
        close(server_socket);
        return -1;
    }

    std::cout << "Client connecté." << std::endl;

    // Fermeture du socket serveur après la connexion
    close(server_socket);

    return client_socket;  // Retourne le socket du client
}
