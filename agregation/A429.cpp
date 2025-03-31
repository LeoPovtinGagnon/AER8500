#include "A429.hpp"
#include <iostream>
#include <cmath>
#include <bitset>  // Inclure la bibliothèque bitset 
#include "agregator.hpp"



// Fonction pour envoyer un message ARINC 429 via TCP en utilisant un socket existant
void sendARINC429Message(int label, uint32_t data, int sock) {
    // Encodage des données selon le label
    uint32_t encodedData = encodeARINC429Message(label, data);

    // Envoyer les données (message ARINC 429)
    ssize_t sentBytes = send(sock, &encodedData, sizeof(encodedData), 0);
    if (sentBytes < 0) {
        std::cerr << "Erreur d'envoi du message" << std::endl;
    } 
}

// Fonction d'encodage des données en fonction du label
uint32_t encodeARINC429Message(int label, int value) {
    uint32_t encodedData = 0;
    // Variables pour encoder les valeurs en BCD
    int bcdValue = 0;
    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;
    switch (label) {
        case 1:  // Altitude

            encodedData |= 0x80; // Label 001
            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            encodedData |= (autopilot & 1) << 8;
            // Encoder l'altitude dans les bits [13:28]
            encodedData |= (value & 0xFFFF) << 12;
            // Bits 30 et 31 à 1 pour indiquer un fonctionnement normal en BNR
            encodedData |= (3 << 29);
            break;
        case 2:  // Taux de montée
           
            // Encodage BCD sur 4 bits 
             encodedData |= 0x40; // Label 002

            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            encodedData |= (autopilot & 1) << 8;

            if (value == 8000) {
                encodedData |= (1 << 13);  //Utilisation du bit 14 pour le cas du taux de montée max (800m/min)
            }
            
            else {
                // Conversion en BCD des chiffres du taux de montée
                bcdValue = value;  
        
                // Extraction des chiffres BCD (en base 10)
                digit1 = (bcdValue / 1000) % 10;  // Chiffre des centaines
                digit2 = (bcdValue / 100) % 10;   // Chiffre des dizaines
                digit3 = (bcdValue / 10) % 10;    // Chiffre des unités
                digit4 = bcdValue % 10;           // Chiffre des dizièmes

                // Placement des chiffres dans encodedData
                encodedData |= (digit1 << 26);  // Bits 29-27 (3 bits)
                encodedData |= (digit2 << 22);  // Bits 26-23 (4 bits)
                encodedData |= (digit3 << 18);  // Bits 22-19 (4 bits)
                encodedData |= (digit4 << 14);  // Bits 18-15 (4 bits)
            }
            break;
        case 3:  // Angle d'attaque
            encodedData |= 0xC0;  // Label 003 

            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            encodedData |= (autopilot & 1) << 8;

            bcdValue = value; 
            // Si l'angle est négatif (-16.0° à -0.1°)
            if (bcdValue < 160) {  
                encodedData |= (3 << 29); // Bit de signe (30) à 1 pour indiquer un angle négatif (on mets aussi le bit 31 à 1 car sinon ça veut dire NCD)
                bcdValue = std::abs(bcdValue - 160);
            }

            // Angle nul ou positif
            else {
                bcdValue = bcdValue - 160;
            }
            // Extraction des chiffres en BCD
            digit1 = (bcdValue / 100) % 10;  // Dizaines
            digit2 = (bcdValue / 10) % 10;   // Unités
            digit3 = bcdValue % 10;          // Dizièmes

            // Placement dans les bits
            encodedData |= (digit1 << 26);  // Bits 29-27
            encodedData |= (digit2 << 22);  // Bits 26-23
            encodedData |= (digit3 << 18);  // Bits 22-19
            break;
        case 10:  // Puissance
            encodedData |= 0x10; //Label 010 

            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            encodedData |= (autopilot & 1) << 8;

            // Encoder la puissance dans les bits [22:28]
            encodedData |= (value & 0xFFFF) << 21;
            // Bits 30 et 31 à 1 pour indiquer un fonctionnement normal en BNR
            encodedData |= (3 << 29);
            break;
        default:
            std::cerr << "Label inconnu, encodage par défaut" << std::endl;
            break;
    }
    // Calculer la parité impaire
    int bitCount = __builtin_popcount(encodedData);  // Compte le nombre de bits à 1
    
    if (bitCount % 2 == 0) {
        // Si la parité est paire, définir le bit de parité à 1 (bit 32)
        encodedData |= 0x80000000;
    }
    return encodedData;

}
void receiveARINC429Message(int sock) {
    uint32_t receivedData;
    // Réception des données via TCP
    ssize_t receivedBytes = recv(sock, &receivedData, sizeof(receivedData), 0);
    if (receivedBytes <= 0) {
        std::cerr << "Erreur de réception du message ARINC 429" << std::endl;
        return;
    }

    // Extraction du label (bits 1 à 8)
    int label = receivedData & 0xFF;
  
  
    // Variables pour le décodage 429 
    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;
    int bcdValue = 0;
    bool isNegative = false;
    bool isMaxRate = false;



    // Affichage du message correspondant
    switch (label) {
        
        case 0x80:
            // Extraire l'altitude
            system_altitude = (receivedData >> 12) & 0xFFFF;  // Extraire les bits de données
            
            // Extraire l'état du système
            currentState = static_cast<avionicState>((receivedData >> 10) & 0x3);
            
            break;
        
        case 0x40: 
            // Vérifier si le taux de montée est maximal (bit 13 à 1)
            isMaxRate = (receivedData & (1 << 13)) != 0;

            // Si le taux de montée est maximal (800 m/min)
            if (isMaxRate) {
                system_climbRate = 800.0;
            } 
            else {
                // Extraire les chiffres encodés en BCD (bits 29-27, 26-23, 22-19, 18-15)
                 digit1 = (receivedData >> 26) & 0x7;  // Chiffre des milliers
                 digit2 = (receivedData >> 22) & 0xF;  // Chiffre des centaines
                 digit3 = (receivedData >> 18) & 0xF;  // Chiffre des dizaines
                 digit4 = (receivedData >> 14) & 0xF;  // Chiffre des unités

            // Reconstituer le taux de montée en BCD
            bcdValue = digit1 * 1000 + digit2 * 100 + digit3 * 10 + digit4;
            system_climbRate = bcdValue / 10.0;

        
             }
            break;
     
        case 0x10:
            // Extraire la puissance
            system_power = (receivedData >> 21) & 0x7F;  // Extraire les bits de données
            break;
        default:
            std::cout << "Label inconnu : " << label << std::endl;
            break;
    }
}