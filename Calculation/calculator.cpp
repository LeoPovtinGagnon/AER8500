#include "server.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "A429.hpp"
#include "calculator.hpp"
bool stagnantPowerFlag = false;
const int MAX_ALTITUDE = 40000;

std::string server_ip = "127.0.0.1";  // L'IP du serveur 
int server_port = 8080;  // Le port du serveur
// Démarre le serveur et récupère le client_socket
int client_socket = start_server(server_ip, server_port);

// État avionique
avionicState currentState = AU_SOL;

// Variables (altitude, taux de montée, angle d'attaque et puissance) actuelles de l'avion
int live_altitude = 0;
float live_climbRate = 0.0;
float live_angle = 0.0;
int live_power = 0;
int live_speed = 0;
// Float pour que l'altitude puisse réagir aux petites variations de climbrate
float preciseAltitude = 0.0;
// Flag qui détermine si le taux de montée et l'angle sont calculés par l'algorithme (autopilot) ou l'agrégateur
bool autopilot = false;

// Mode d'exécution (1 sec = 1 min si true, sinon temps réel)
bool use_seconds = true;

// Thread qui met à jour l'altitude
void altitudeUpdater() {
    while (true) {
        double time_factor = use_seconds ? 1.0 : (1.0 / 60.0);  // Mode accéléré ou temps réel
        preciseAltitude += live_climbRate * time_factor;
        live_altitude = static_cast<int>(round(preciseAltitude));
        std::cout<<live_altitude<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Fonction qui détecte les changements d'états
void stateManager(){
    switch (currentState) {
        case AU_SOL:
            if (desired_altitude != 0 && desired_angle == 0 && desired_climbRate == 0) {
                autopilot = true;
                currentState = CHANGEMENT_ALT;
            }
            if (desired_altitude == 0 && desired_angle != 0 && desired_climbRate != 0){
                currentState = CHANGEMENT_ALT;
            }
            break;
        case CHANGEMENT_ALT:
            
            if(autopilot && live_altitude == desired_altitude && stagnantPowerFlag){
                currentState = VOL_CROISIERE;
                live_climbRate = 0;
            }
            if(live_altitude == MAX_ALTITUDE && !autopilot){
                currentState = VOL_CROISIERE;
                
            }
            if(!autopilot && desired_climbRate==0){
                currentState = VOL_CROISIERE;
            }
            break;
        default:
            std::cout << currentState << std::endl;
            std::cout << "Autopilote: " << autopilot << std::endl;
            break;
    }
}

// Fonction qui envoie les plus récentes valeurs à l'agrégateur
void dispatchValues(int client_socket){
    sendARINC429Message(1, live_altitude, client_socket);
    sendARINC429Message(2, static_cast<int>(round(desired_climbRate * 10)), client_socket); // Encodage 429
    sendARINC429Message(10, live_power, client_socket);
}

double sinDeg(double degrees) {
    return sin(degrees * M_PI / 180.0); // Conversion en radians
}

int powerManager(){
    int calculatedPower = 0;
    std::cout<<"tigre: "<<desired_angle<<std::endl;
    float angle = desired_angle;
    if(desired_angle < 0.0){
        angle = -desired_angle;
    }
    // Prévention du décrochage
    if(desired_angle > 15.0){
        calculatedPower = round(live_climbRate/(10*sinDeg(15.0)+0.0001)); // On ajoute un petit facteur pour éviter les divisions par 0
    }
    // Cas général
    else{
        calculatedPower = round(live_climbRate/(10*sinDeg(angle)+0.0001)); 
        if(calculatedPower > 100){
           
        }
    

    }
    std::cout<<"pandore: "<<angle<<std::endl;
    return calculatedPower;


    
}
// Le chef d'orchestre de ce magnifique code 
void calculatorProcess(){
    if(autopilot){
        live_altitude = desired_altitude;
        live_power = desired_power;
    }
    else{
        // To do altitude sans autopilote
        live_climbRate = desired_climbRate;
        live_angle = desired_angle;
        live_power = powerManager();
    }
}

int main() {
    std::thread altitudeThread(altitudeUpdater);  // Démarrage du thread d'altitude
    altitudeThread.detach();  // On détache pour qu'il fonctionne en arrière-plan

    // Boucle d'exécution principale
    while (true) {
        receiveARINC429Message(client_socket);
        stateManager();
        calculatorProcess();
        dispatchValues(client_socket);
    }
    return 0;
}

