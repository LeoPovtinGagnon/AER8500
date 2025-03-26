#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

void start_client(const std::string& server_ip, int server_port) {
    // Création du socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return;
    }

    // Paramétrage de l'adresse du serveur
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr)); // Initialiser à 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Adresse IP invalide." << std::endl;
        close(client_socket);
        return;
    }

    // Connexion au serveur
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connexion au serveur échouée." << std::endl;
        close(client_socket);
        return;
    }

    std::cout << "Connecté au serveur." << std::endl;


}