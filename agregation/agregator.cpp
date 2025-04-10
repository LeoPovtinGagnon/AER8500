#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include "client.hpp"
#include "A429.hpp"
#include "agregator.hpp"
#include "UI.hpp"
#include <atomic>

// Variables transmises par le calculateur
int system_altitude = 0;
float system_climbRate = 0.0;
float system_angle = 0.0;
int system_power = 0;

// État avionique
avionicState currentState = AU_SOL;

// Constantes pour la connexion TCP
std::string server_ip = "127.0.0.1";
int server_port_429 = 8080;
int server_port_AFDX = 8081;
int server_socket_429 = start_client(server_ip, server_port_429);
int server_socket_AFDX = -1;

void client_thread_AFDX(const std::string& server_ip, int server_port_AFDX) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    server_socket_AFDX = start_client(server_ip, server_port_AFDX);
    std::cout<<"afdx"<<std::endl;
    if (server_socket_AFDX >= 0) {
        std::cout << "Connecté au serveur AFDX." << std::endl;
      
    }
}

// Flag pour l'activation et la désactivation de l'autopilote
bool autopilot = false;
// Flags pour la gestion de l'atterissage
bool landing_flag = false;

// Flag indiquant une utilisation problématique de la puissance (normalement faux)
bool power_problems_flag = false;

// Flag pour veiller à ce que l'angle d'attaque soit positif au décolage (faux si angle  négatif)
bool takeoffAngle_flag = true;

// Variable pour arrêter le thread de réception
bool receiving = true;


// Thread de réception des données du calculateur
void receiveARINC429Thread(int server_socket_429) {
    while (receiving) {
        receiveARINC429Message(server_socket_429);
    }
}

// Thread de d'envoi des commandes par ARINC429
void sendARINC429(int server_socket_429) {
    
  
        // Envoi des commandes les plus récentes
        // Altitude
        sendARINC429Message(1, altitude_computed, server_socket_429 );
        // Taux de montée
        sendARINC429Message(2, ascentRate_computed, server_socket_429 );
        // Angle
        sendARINC429Message(3, angle_computed, server_socket_429 );
        // Puissance
        sendARINC429Message(10, power_computed, server_socket_429 );
          
    
}

// Fonction qui gère le décolage
void takeoffManager(){

    if(landing_flag){
        cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", 0);
        cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", 160);
        cv::setTrackbarPos("Puissance", "Panneau de controle", 0);
        cv::setTrackbarPos("Altitude                  ", "Panneau de controle", 0);
        autopilot = false;
    }
    if(currentState == AU_SOL & altitude_computed != 0 & ascentRate_computed == 0 & angle_computed == 160){
        autopilot = true;
        takeoffAngle_flag = true;
    }
    // Angle d'attaque de décolage négatif: invalide
    if(currentState == AU_SOL & altitude_computed == 0 & ascentRate_computed != 0 & angle_computed < 160){
        takeoffAngle_flag = false;
    }
    // Angle d'attaque positif au décolage: valide
    if(currentState == AU_SOL & altitude_computed == 0 & ascentRate_computed != 0 & angle_computed > 160){

        takeoffAngle_flag = true;
    }
  
}


// Fonction qui assure la continuité des commandes lors du passage entre autopilote et pilotage manuel
void transitionManager(){
   
    if(currentState != AU_SOL){
        if(autopilot){
             cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", system_climbRate * 100);
             cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle",(static_cast<int>(((system_angle) * 10.0)) + 160));
        }
        else {
            cv::setTrackbarPos("Puissance", "Panneau de controle", system_power);
            cv::setTrackbarPos("Altitude                  ", "Panneau de controle", system_altitude);
        }
    }
  
}


int main() {

    std::thread t2(client_thread_AFDX, server_ip, server_port_AFDX); // Client AFDX
    // Thread pour recevoir les données du calculateur
    std::thread receive429Thread(receiveARINC429Thread, server_socket_429);

    UI_init();

    while (true) {
        // Mise à jour  de l'UI
        cv::Mat image;
        UI_Process(image);
        takeoffManager();
        transitionManager();
        sendARINC429(server_socket_429);
        

        cv::imshow("Panneau de controle", image);
  
        // Sortir si l'utilisateur ferme la fenêtre
        if (cv::waitKey(1) == 27) {
            receiving = false;
            
            break;
        }
    }
    t2.join();
    receive429Thread.join();
    cv::destroyAllWindows();
    close(server_socket_429);
    close(server_socket_AFDX);
    return 0;
}