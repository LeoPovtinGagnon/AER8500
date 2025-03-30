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
int system_power = 0;

// État avionique
avionicState currentState = AU_SOL;

// Constantes pour la connexion TCP
std::string server_ip = "127.0.0.1";
int server_port = 8080;
int client_socket = start_client(server_ip, server_port);

// Flag pour l'activation et la désactivation de l'autopilote
bool autopilot = false;


// Variable pour arrêter le thread de réception
bool receiving = true;
int received_value = 0;

// Thread de réception des données du calculateur
void receiveARINC429Thread(int client_socket) {
    while (receiving) {
        receiveARINC429Message(client_socket);
    }
}




// Fonction qui va envoyer les inputs du pilote
void ValueUpdate() {

    if(currentState == AU_SOL & altitude_computed != 0 & ascentRate_computed == 0 & angle_computed == 160){
        autopilot = true;
    }
    // Envoi des commandes les plus récentes
    // Altitude
    sendARINC429Message(1, altitude_computed, client_socket );
    // Taux de montée
    sendARINC429Message(2, ascentRate_computed, client_socket );
    // Angle
    sendARINC429Message(3, angle_computed, client_socket );
    // Puissance
    sendARINC429Message(10, power_computed, client_socket );
          
}




int main() {

    // Thread pour recevoir les données du calculateur
    std::thread receiveThread(receiveARINC429Thread, client_socket);
    UI_init();

    while (true) {
        // Mise à jour  de l'UI
        cv::Mat image;
        UI_Process(image);
        cv::imshow("Panneau de controle", image);

        // Envoi péridiquement les entrées les plus récentes de l'utilisateur
        ValueUpdate();
        

        // Sortir si l'utilisateur ferme la fenêtre
        if (cv::waitKey(1) == 27) {
            receiving = false;
            
            break;
        }
    }
    close(client_socket);
    cv::destroyAllWindows();
    receiveThread.join();
    return 0;
}