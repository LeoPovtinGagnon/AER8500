#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int start_client(const std::string& server_ip, int server_port) {
    // Création du socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    // Paramétrage de l'adresse du serveur
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr)); // Initialiser à 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Adresse IP invalide." << std::endl;
        close(client_socket);
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    // Connexion au serveur
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connexion au serveur échouée." << std::endl;
        close(client_socket);
        return -1;  // Retourne une valeur négative pour signaler une erreur
    }

    std::cout << "Connecté au serveur." << std::endl;

    return client_socket;  // Retourne le client_socket si tout est ok
}


int receiveInt(int client_socket) {
    int received_value;
    ssize_t result = recv(client_socket, &received_value, sizeof(received_value), 0);  // Utilise directement l'adresse de received_value

    if (result == -1) {
        std::cerr << "Erreur lors de la réception de l'entier." << std::endl;
        return -1;  // Retourne -1 si une erreur s'est produite
    }
    else if (result == 0) {
        std::cerr << "Connexion fermée par le serveur." << std::endl;
        return -1;
    }

    return received_value;
}