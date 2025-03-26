#include "server.hpp"
#include <iostream>
#include <thread>
#include <chrono>

std::string server_ip = "127.0.0.1";  // L'IP du serveur 
int server_port = 8080;  // Le port du serveur

int main() {
    // Démarre le serveur et récupère le client_socket
    int client_socket = start_server(server_ip, server_port);
    if (client_socket == -1) {
        std::cerr << "Échec de la connexion au client." << std::endl;
        return -1;
    }

    // Boucle d'exécution principale
    while (true) {
        // Envoie un entier aléatoire au client toutes les secondes
        int random_value = rand() % 100;  // Générer un entier aléatoire entre 0 et 99
        sendInt(client_socket, random_value);
        
        // Attendre 1 seconde avant d'envoyer le prochain entier
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    
    return 0;
}