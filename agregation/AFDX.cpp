#include "agregator.hpp"
#include "UI.hpp"
#include <cmath>
#include <arpa/inet.h>
#include "A429.hpp"

int sentSequence = 0;
int receivedSequence = 0;



// On se contente d'envoyer le payload et un numéro de séquence
void sendAFDXMessage(int sock) {
    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;
    bool isNegative = false;
    //Assure le padding des octets inutilisés 
    uint8_t message[18] = {0};

     //Numéro de séquence
     message[0]= sentSequence;
     sentSequence++;
     if(sentSequence == 256){
        sentSequence = 1;
     }

    // Codage de l'altitude sur 16 bits
    message[1] = altitude_computed & 0xFF;         // LSB altitude
    message[2] = (altitude_computed >> 8) & 0xFF;  // MSB altitude
    
    // Codage du taux de montée
    // Conversion en BCD des chiffres du taux de montée
    int climbRateValue = ascentRate_computed;
    // Extraction des chiffres (on garde l'encodage en bcd, mais avec 4 bits pour les centaines)
    digit1 = (climbRateValue / 1000) % 10;  // Chiffre des centaines
    digit2 = (climbRateValue / 100) % 10;   // Chiffre des dizaines
    digit3 = (climbRateValue / 10) % 10;    // Chiffre des unités
    digit4 = climbRateValue % 10;           // Chiffre des dizièmes
    // Placement du taux de montée dans le message
    message[3] = (digit3 << 4) | (digit4 & 0x0F);
    message[4] = (digit1 << 4) | (digit2 & 0x0F);
    


    // Codage de l'angle et du flag d'autopilot
    int angleValue = angle_computed; 
    // Si l'angle est négatif (-16.0° à -0.1°)
    if (angleValue < 160) {  
        isNegative = true;
        angleValue = std::abs(angleValue - 160);
    }

    // Angle nul ou positif
    else {
        isNegative = false;
        angleValue = angleValue - 160;
    }

    // Extraction des chiffres de l'angle en BCD 
    digit1 = (angleValue / 100) % 10;  // Dizaines
    digit2 = (angleValue / 10) % 10;   // Unités
    digit3 = angleValue % 10;          // Dizièmes
    // Encodage du bit de signe et du flag indiquant l'autopilote 
    uint8_t flags = 0;
    flags |= (isNegative ? 1 : 0);        // Bit 0
    flags |= (autopilot ? 1 : 0) << 1;    // Bit 1

    // Placement de l'angle et du flag d'autoplilote dans le message
    message[5] = (digit2 << 4) | (digit3 & 0x0F);
    message[6] = (flags << 4) | (digit1 & 0x0F);

   // Codage de la puissance 
    message[7] = power_computed;
   
   // Envoi du message AFDX au calculateur
    ssize_t sentBytes = send(sock, message, sizeof(message), 0);
 
}

// Réception des données AFDX du calculateur
void receiveAFDXMessage(int sock) {

    // Variables intermédiaires pour manipuler les données
    int state = 0;
    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;
    bool isNegative = false;

    // Tableau de données reçu
    uint8_t message[18] = {0};


    if (!protocolSelector) { // L'AFDX prend le relais en cas de problème avec le ARINC429 (redondance)
        std::cout<<"Réception Via AFDX"<<std::endl;
        // Réception du message AFDX de l'agrégateur
        ssize_t receivedBytes = recv(sock, message, sizeof(message), 0);

        // Numéro de séquence du message
        receivedSequence = message[0];

        // Altitude 
        system_altitude =  message[1] | (message[2] << 8);

        // Taux de montée
        digit1 = (message[4] >> 4) & 0x0F;
        digit2 = message[4] & 0x0F;
        digit3 = (message[3] >> 4) & 0x0F;
        digit4 = message[3] & 0x0F;
        system_climbRate = (digit1 * 1000 + digit2 * 100 + digit3 * 10 + digit4)/10.0;

        // Angle et état avionique
        digit1 = message[6] & 0x0F;             // Dizaines
        digit2 = (message[5] >> 4) & 0x0F;      // Unités
        digit3 = message[5] & 0x0F;             // Dixièmes

        // Extraire le signe de l'angle
        isNegative = (message[6] >> 4) & 0x01;
        // Extraire l'état avionique 
        state = (message[6] >> 5) & 0x03;
        // Cas de l'atterissage
        if(state == 3){

            landing_flag = true;
            // L'avion est de retour au sol
            currentState = AU_SOL;
            
        }

        // Gestion des états standards
        else{

            landing_flag = false;
            currentState = static_cast<avionicState>((message[6] >> 5) & 0x03);
        }
        // Décoder l'angle
        float angle = (digit1 * 100 + digit2 * 10 + digit3)/10.0;
        if(isNegative){
            system_angle = -angle;
        }
        else {
            system_angle = angle;
        }

        // Puissance
        system_power = message[7];

        // Détection des problèmes liés à la puissance
        if(message[8] == 1){
            power_problems_flag = true;
        }
        // Cas normal: pas de problème
        else{
            power_problems_flag = false;
        }
    }
}
