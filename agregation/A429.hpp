#include <cstdint>
#include <sys/socket.h>  // Pour le type socket

// Fonction pour envoyer un message ARINC 429 via TCP
void sendARINC429Message(int label, uint32_t data, int sock);

// Fonction pour encoder les données en fonction du label
uint32_t encodeARINC429Message(int label, int value);
// Fonction de réception et de décodage des messages du calculateur
void receiveARINC429Message(int sock);

