#include "calculator.hpp"
#include "A429.hpp"
#include <cmath>
#include <arpa/inet.h>
#include <iostream>

int sequence = 0;



// On se contente d'envoyer le payload et un numéro de séquence
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

        sequence = message[0];
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
        desired_power = message[7];
         std::cout<<"alt :" <<desired_altitude<<std::endl;
          std::cout<<"pilo :" <<autopilot<<std::endl;
        
    
   

        


}
    

    


