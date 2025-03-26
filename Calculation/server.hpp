
#include <string>

// Déclare la fonction qui démarre le serveur
int start_server(const std::string& server_ip, int server_port);
// Envoie des int
void sendInt(int socket, int value);