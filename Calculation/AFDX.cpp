#include "calculator.hpp"
#include "A429.hpp"
#include <cmath>
#include <arpa/inet.h>
#include <iostream>


// Numéros de séquence
int receivedSequence = 0;
int sentSequence = 0;



void receiveAFDXMessage(int sock) {
    
    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;
    bool isNegative = false;

    // Tableau de données reçu
    uint8_t message[18] = {0};

    // Réception du message AFDX de l'agrégateur
    ssize_t receivedBytes = recv(sock, message, sizeof(message), 0);
    if(!protocolSelector){
        std::cout<<"Réception Via AFDX"<<std::endl;
        receivedSequence = message[0];
        // Altitude 
        desired_altitude =  message[1] | (message[2] << 8);
        // Taux de montée
        digit1 = (message[4] >> 4) & 0x0F;
        digit2 = message[4] & 0x0F;
        digit3 = (message[3] >> 4) & 0x0F;
        digit4 = message[3] & 0x0F;
        desired_climbRate = (digit1 * 1000 + digit2 * 100 + digit3 * 10 + digit4)/10.0;
        // Angle et autopilote
        digit1 = message[6] & 0x0F;             // Dizaines
        digit2 = (message[5] >> 4) & 0x0F;       // Unités
        digit3 = message[5] & 0x0F;              // Dixièmes
        isNegative = (message[6] >> 4) & 0x01;
        autopilot = (message[6] >> 5) & 0x01;
        float angle = (digit1 * 100 + digit2 * 10 + digit3)/10.0;
        if(isNegative){
            desired_angle = -angle;
        }
        else {
            desired_angle = angle;
        }
        // Puissance
        // On vérifie si la puissance est stable
        if(desired_power == message[7]){

            stagnantPowerFlag = true;
        }  
        else {

            stagnantPowerFlag = false;
        }

        desired_power = message[7];
    }   
  
}



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
    message[1] = live_altitude & 0xFF;         // LSB altitude
    message[2] = (live_altitude >> 8) & 0xFF;  // MSB altitude
    // Codage du taux de montée 
    int climbrateValue = static_cast<int>(live_climbRate * 10);  
    digit1 = (climbrateValue / 1000) % 10;  // Centaines
    digit2 = (climbrateValue / 100) % 10;   // Dizaines
    digit3 = (climbrateValue / 10) % 10;    // Unités
    digit4 = climbrateValue % 10;           // Dixièmes
    // Insertion dans le message
    message[3] = (digit3 << 4) | digit4;  // Octet = [digit1][digit2]
    message[4] = (digit1 << 4) | digit2;  // Octet = [digit3][digit4]

    // Codage de l'angle
    int angleValue = static_cast<int>(round(live_angle * 10));
    if(angleValue < 0){
        angleValue = -angleValue;
        isNegative = true;
    }
    else{
        isNegative = false;
    }
    // Bit de signe et de l'état avionique
    uint8_t flags = 0;
    flags |= (isNegative ? 1 : 0);        // Signe de l'angle
    flags |= ((currentState & 0x3)) << 1;    //État
    // Extraction des chiffres de l'angle
    digit1 = (angleValue / 100) % 10;  // Dizaines
    digit2 = (angleValue / 10) % 10;   // Unités
    digit3 = angleValue % 10;          // Dizièmes
    // Placement de l'angle et de l'état avionique dans le message
    message[5] = (digit2 << 4) | (digit3 & 0x0F);
    message[6] = (flags << 4) | (digit1 & 0x0F);

    // Codage de la puissance
    message[7] = live_power;

    // Utilisation de l'octet 9 pour indiquer des problèmes de puissance à l'agrégateur
    if((notEnoughPowerFlag || tooMuchPowerFlag)){
        message[8] = 1;
    }

    // Envoi du message AFDX à l'agrégateur
    ssize_t sentBytes = send(sock, message, sizeof(message), 0);
    
}

  
    


    

    


