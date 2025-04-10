#include <iostream>
#include <cstdint>
#include <sys/socket.h>
#include <stdio.h>
#include <cmath>
#include <bitset>
#include  "calculator.hpp" 
#include "A429.hpp"

// Données de l'agrégateur
uint32_t desired_altitude = 0;
uint32_t desired_power = 0;
float desired_angle = 0.0;
float desired_climbRate = 0.0;
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
            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            autopilot = (receivedData)>>8 & 1;
            
            // Extraire l'altitude
            desired_altitude = (receivedData >> 12) & 0xFFFF;  // Extraire les bits de données
            
            break;
        
        case 0x40: 

            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            autopilot = (receivedData)>>8 & 1;

            // Vérifier si le taux de montée est maximal (bit 13 à 1)
            isMaxRate = (receivedData & (1 << 13)) != 0;

            // Si le taux de montée est maximal (800 m/min)
            if (isMaxRate) {
                desired_climbRate = 800.0;
            } 
            else {
                // Extraire les chiffres encodés en BCD (bits 29-27, 26-23, 22-19, 18-15)
                 digit1 = (receivedData >> 26) & 0x7;  // Chiffre des milliers
                 digit2 = (receivedData >> 22) & 0xF;  // Chiffre des centaines
                 digit3 = (receivedData >> 18) & 0xF;  // Chiffre des dizaines
                 digit4 = (receivedData >> 14) & 0xF;  // Chiffre des unités

            // Reconstituer le taux de montée en BCD
            bcdValue = digit1 * 1000 + digit2 * 100 + digit3 * 10 + digit4;
            desired_climbRate = bcdValue / 10.0;

        
             }
            break;
        case 0xC0:

            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            autopilot = (receivedData)>>8 & 1;

            // Extraire le bit de signe (bit 30)
            isNegative = (receivedData & (1 << 29)) != 0;

            // Extraire les chiffres encodés en BCD (bits 29-27, 26-23, 22-19)
            digit1 = (receivedData >> 26) & 0x7;  // Chiffre des dizaines
            digit2 = (receivedData >> 22) & 0xF;  // Chiffre des unités
            digit3 = (receivedData >> 18) & 0xF;  // Chiffre des dixièmes

            // Reconstituer l'angle en BCD
            bcdValue = digit1 * 100 + digit2 * 10 + digit3;
            desired_angle = bcdValue / 10.0; //Passage en float

            // Si l'angle est négatif, appliquer la correction
            if (isNegative) {
                desired_angle = -1 * desired_angle;
            }

            break;
        case 0x10:
            // Bits de SDI (0: pilotage manuel, 1: autopilote)
            autopilot = (receivedData)>>8 & 1;

            // Extraire la puissance
            if(desired_power == ((receivedData >> 21) & 0x7F)){
                stagnantPowerFlag = true;
            }  // Extraire les bits de données
            else {
                stagnantPowerFlag = false;
            }
            desired_power = (receivedData >> 21) & 0x7F;
            break;
        default:
            std::cout << "Label inconnu : " << label << std::endl;
            break;
    }
}


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

            encodedData |= 0x80; //Label 001 
            // Encoder l'altitude dans les bits [13:28]
            encodedData |= (value & 0xFFFF) << 12;
            // Encodage de l'état 
            encodedData |= (currentState & 0x3) << 10;
            // Bits 30 et 31 à 1 pour indiquer un fonctionnement normal en BNR
            encodedData |= (3 << 29);
            break;

        case 2:  // Taux de montée
           
            // Encodage BCD sur 4 bits 
            encodedData |= 0x40; // Label 002

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

            bcdValue = value; 
            // Si l'angle est négatif (-16.0° à -0.1°)
            if (bcdValue < 0.0) {  
                encodedData |= (3 << 29); // Bit de signe (30) à 1 pour indiquer un angle négatif (on mets aussi le bit 31 à 1 car sinon ça veut dire NCD)
                bcdValue = std::abs(bcdValue);
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

            // Encoder la puissance dans les bits [22:28]
            encodedData |= (value & 0xFFFF) << 21;

            // Utilisation des bits SSM pour indiquer une puissance invalide
            if(!(notEnoughPowerFlag || tooMuchPowerFlag)){

            // Bits 30 et 31 à 1 pour indiquer un fonctionnement normal en BNR
            encodedData |= (3 << 29);

            }

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
