#include "server.hpp"
#include <iostream>

std::string server_ip = "127.0.0.1";  // L'IP du serveur 
int server_port = 8080;  // Le port du serveur

int main() {
    
    // Démarre le serveur
    start_server(server_ip, server_port);

    // Boucle d'exécution principale
    while(1){
    
    }

    return 0;
}