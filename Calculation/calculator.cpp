#include "server.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "A429.hpp"
#include "calculator.hpp"
#include <stdlib.h>  
#include <cmath>
#include <atomic>
#include <unistd.h> 



// Valeurs max
const int MAX_ALTITUDE = 40000;
const float MAX_CLIMBRATE = 800.0;
const int MAX_POWER = 100;


// Flag qui indique que l'avion effectue une descente
bool descendingFlag = false;


// Détection d'une puissance stable pour le passage à l'état de croisière
bool stagnantPowerFlag = false;
// Détection d'une puissance inutilement élevée (taux de montée max atteint)
bool tooMuchPowerFlag = false;
// Détection d'une puissance insuffisante
bool notEnoughPowerFlag = false;



// Variable pour arrêter le thread de réception
bool receiving = true;


std::string server_ip = "127.0.0.1";  // L'IP du serveur 
int server_port_429 = 8080;  // Le port du serveur pour ARINC429
int server_port_AFDX = 8081; // Port serveur pour AFDX

// Démarre le serveur et récupère le client_socket_429
int client_socket_429 = start_server(server_ip, server_port_429);
int client_socket_AFDX = -1;
void server_thread_AFDX(const std::string& server_ip, int server_port_AFDX) {
    client_socket_AFDX = start_server(server_ip, server_port_AFDX);
    if (client_socket_AFDX >= 0) {
        std::cout << "Connexion AFDX établie." << std::endl;
        // Gérer la communication AFDX ici
        // Utilise client_socket_AFDX pour recevoir/envoyer des données
    }
}

// État avionique
avionicState currentState = AU_SOL;

// Variables (altitude, taux de montée, angle d'attaque et puissance) actuelles de l'avion
int live_altitude = 0;
float live_climbRate = 0.0;
float live_angle = 0.0;
int live_power = 0;

// Float pour que l'altitude puisse réagir aux petites variations de climbrate
float preciseAltitude = 0.0;
float preciseClimbRate = 0.0;
float precise_angle = 0.0;

// Flag qui détermine si le taux de montée et l'angle sont calculés par l'algorithme (autopilot) ou l'agrégateur
bool autopilot = false;

// Mode d'exécution (1 sec = 1 min si true, sinon temps réel)
bool use_seconds = true;

// Thread qui monitor l'altitude actuelle et réagit aux différentes situations
void altitudeMonitor() {
    while (true) {
        // Détecter si l'altitude max ou désirée est atteinte
        if ((live_altitude == desired_altitude && autopilot) || 
            (live_altitude >= MAX_ALTITUDE && !autopilot) || (live_altitude <= 0)) {
            stateManager();  
        }
        if(currentState == VOL_CROISIERE && desired_angle < 0.0 && !autopilot){
            std::cout<<"coc"<<std::endl;
            stateManager();
        }

        if(currentState != VOL_CROISIERE){
        // Mode autopilote (commande de l'altitude et de la puissance)
        if(autopilot){
            // Descente
            if (live_altitude > desired_altitude){
        
                descendingFlag = true;
            }
            // Montée
            else {

            descendingFlag = false;

            }
            // Appel le process qui gère l'angle d'attaque et le taux de montée en fonction de l'altitude et de la puissance
            autopilotManager();      
        }
        // Mode manuel (commande directe de l'angle d'attaque et du taux de montée)
        else{
            // Descente
            if(live_angle < 0.0){
                
                descendingFlag = true;
            }
            // Montée
            else{

                descendingFlag =false;
            }
            // Appel le process qui gère le pilotage manuel
            manualManager();
        }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// Thread qui met à jour l'altitude
void altitudeUpdater() {
    while (true) {
      
            double time_factor = use_seconds ? 1.0 : (1.0 / 60.0);  // Mode accéléré ou temps réel
            
            if(currentState != VOL_CROISIERE && currentState != LANDING){
            // Perte d'altitude
            if (descendingFlag){
        
                preciseAltitude -= preciseClimbRate * time_factor * 3.28084; // conversion mètres vers pieds
            }
            // Prise d'altitude
            else{

                preciseAltitude += preciseClimbRate * time_factor * 3.28084;
            }
            live_altitude = static_cast<int>(round(preciseAltitude));
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));       
        }

  
}

// Fonction qui détecte les changements d'états
void stateManager(){
    switch (currentState) {
        case AU_SOL:
            if (desired_altitude != 0 && desired_angle == 0 && desired_climbRate == 0) {
                currentState = CHANGEMENT_ALT;
            }
            if (desired_altitude == 0 && desired_angle != 0 && desired_climbRate != 0 && live_altitude != 0){
                currentState = CHANGEMENT_ALT;
               
            }
            break;
        case CHANGEMENT_ALT:
            
            if(autopilot && live_altitude == desired_altitude && stagnantPowerFlag){
                preciseClimbRate = live_climbRate = 0.0;
                precise_angle = live_angle = 0.0;
                
                currentState = VOL_CROISIERE;
            }
            if(live_altitude >= MAX_ALTITUDE && !autopilot && desired_angle >= 0.0){
                preciseClimbRate = live_climbRate = 0.0;
                live_altitude = preciseAltitude = MAX_ALTITUDE;
                currentState = VOL_CROISIERE;
                
            }
            if(!autopilot && desired_climbRate == 0.0 ){
                currentState = VOL_CROISIERE;
            }
            // Atterissage: On mets tout à 0 pour immobiliser notre avion
            if((live_altitude <= 0 && desired_altitude == 0) || (!autopilot && desired_angle < 0.0 && live_altitude <=0 )){
                preciseAltitude = live_altitude = 0;
                preciseClimbRate = live_climbRate = 0.0;
                live_power = 0;
                precise_angle = live_angle = 0.0;
                currentState = LANDING; 
                
            }
            break;
        case VOL_CROISIERE:
            if ((desired_altitude != live_altitude && autopilot) || (autopilot && !stagnantPowerFlag)){
                currentState = CHANGEMENT_ALT;
            }
            if (!autopilot && desired_angle < 0.0 && desired_climbRate !=0 ){
                currentState = CHANGEMENT_ALT;
            }
            break;
        // État bonus pour pouvoir attérir
        case LANDING:
            if(desired_altitude == 0 && desired_angle == 0.0 && desired_power == 0 && desired_climbRate == 0.0){
                currentState = AU_SOL;
            }
            break;
        default:

            break;
    }
}


// Fonction qui envoie les plus récentes valeurs à l'agrégateur
void SendValuesARINC429(int client_socket_429){
    sendARINC429Message(1, live_altitude, client_socket_429);
    sendARINC429Message(2, static_cast<int>(round(live_climbRate * 10)), client_socket_429); // Encodage 429
    sendARINC429Message(10, live_power, client_socket_429);
    sendARINC429Message(3, static_cast<int>(round(live_angle * 10)), client_socket_429);

    
}

// Lorsque pas en autopilote, cette fonction calcule la puissance du moteur et valide l'angle et le taux de montée spécifiés
void manualManager(){

    // Prévention du décrochage
    if(desired_angle > 15.0){
        precise_angle = live_angle = 14.9;
    }

    else{
        precise_angle = live_angle = desired_angle;
    }

    // Limitation du taux de montée si la manoeuvre demande trop de puissance
    if(desired_climbRate > ((abs(live_angle)/16.0) * MAX_POWER * 10.0 )){

        live_power = MAX_POWER;
        preciseClimbRate = live_climbRate = abs(live_angle)/16.0 * live_power * 10.0;
        notEnoughPowerFlag = true; // Ce flag va indiquer au pilote d'augmenter l'angle d'attaque (positivement ou négativement selon le contexte)
    }
    else {

        preciseClimbRate = live_climbRate = desired_climbRate;
        live_power = static_cast<int>(round((live_climbRate / ((abs(live_angle) / 16.0)) / 10.0)));
        notEnoughPowerFlag = false;

    }
   
}  

// Fonction qui calcule l'angle d'attaque et le taux de montée automatiquement
// Pour simplifier, on considère que le taux de montée est proportionnel à l'angle d'attaque et à la vitesse (puissance)
void autopilotManager(){
    // Mise à jour de la puissance actuelle avec l'entrée de l'agrégateur
    live_power = desired_power;
    // Mesure de l'écart  entre l'altitude actuelle  et l'altitude désirée
    int altitudeGap = desired_altitude-live_altitude;
    altitudeGap = abs(altitudeGap);

    // Descente
    if(descendingFlag){
       
        // Si l'écart est plus que 2000 pieds, on maximise (négativement) l'angle pour maximiser la montée (descente)
        if(altitudeGap > 3000){

            precise_angle = -16.0;
        }
        // Ajustement au pied près
        else if(altitudeGap < 5){
                
            precise_angle = (0.3048 * -16.0)/(live_power*10);
        }
        // Ralentissement en approche de l'altitude désirée
        else {
            
            double ratio = altitudeGap / 3000.0;
            precise_angle = -16.0 * pow(ratio, 1.3);  // Fonction d'adoucissement (exposant >1 pour ralentir)
          
        
        }
    }
    else {

        // Si l'écart est plus que 2000 pieds, on maximise l'angle pour maximiser la montée
        if(altitudeGap > 3000){

            // Fixer l'angle maximal juste en dessous de 15 degrées pour éviter le décrochage
            precise_angle = 14.9;

        }
        // Ajustement au pied près
        else if(altitudeGap < 5){
                
            precise_angle = (0.3048 * 16.0)/(live_power*10);
        }
        // Ralentissement en approche de l'altitude désirée
        else {
            
            double ratio = altitudeGap / 3000.0;
            precise_angle = 14.9 * pow(ratio, 1.3);  // Fonction d'adoucissement
        
        }
    }

    // Mise à jour de l'angle actuel (résolution de 0.1 degrées)
    live_angle = round(precise_angle * 10.0) / 10.0;

  

    // Mise à jour du taux de montée (modèle simplifié)
    if((live_power*10*(abs(precise_angle)/16.0)) > 800.0){ // Si le taux de montée dépasse la valeur max, elle est fixée à la valeur max

        preciseClimbRate= live_climbRate = MAX_CLIMBRATE;
        tooMuchPowerFlag = true; // Flag pour avertir le pilote que la puissance peut être diminuée

    }
    else{
        tooMuchPowerFlag = false;
        preciseClimbRate = live_power * 10 * (abs(precise_angle) / 16.0);
        live_climbRate = round(live_power * 10 * (abs(precise_angle) / 16.0) * 10.0) / 10.0;
    }
            
 }


// Thread de réception des données du calculateur
void receiveARINC429(int client_socket_429) {
    while (receiving) {
        receiveARINC429Message(client_socket_429);
    }
}

int main() {
    std::thread altitudeThread(altitudeUpdater);  // Démarrage du thread d'altitude
    altitudeThread.detach();  // On détache pour qu'il fonctionne en arrière-plan
    std::thread altitudeMonitorThread(altitudeMonitor);
    altitudeMonitorThread.detach();
    std::thread receiveARINC429Thread(receiveARINC429, client_socket_429);
    std::thread t2(server_thread_AFDX, server_ip, server_port_AFDX); // Serveur AFDX

    // Boucle d'exécution principale
    while (true) {

        stateManager();
    
        SendValuesARINC429(client_socket_429);
      

    }
    receiving = false;
    t2.join();
    receiveARINC429Thread.join();
    close(client_socket_429);
    close(client_socket_AFDX);
    return 0;
}

