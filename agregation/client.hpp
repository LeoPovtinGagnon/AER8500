#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
int start_client(const std::string& server_ip, int server_port);
int receiveInt(int client_socket);  // Déclaration de la fonction de réception