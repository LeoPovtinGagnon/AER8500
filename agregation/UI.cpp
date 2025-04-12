#include <opencv2/opencv.hpp>
#include "agregator.hpp"
#include "UI.hpp"

// Valeurs maximales possibles des entrées
const int ASCENTRATE_RANGE = 8000;
const int ALTITUDE_RANGE = 40000;
const int ANGLE_RANGE = 320;
const int POWER_RANGE = 100;

// Entrées de l'utilisateur
int altitude_computed = 0;
int ascentRate_computed = 0;
int angle_computed = 160;
int power_computed = 0;

// Affichage textuelles des commandes sur le UI
int altitude_UI = 0;
float ascentRate_UI = 0;
float angle_UI = 0;
int power_UI = 0;

// Ouverture de la fenêtre et génération du UI
void UI_init(){

    cv::namedWindow("Panneau de controle", cv::WINDOW_NORMAL);
    cv::resizeWindow("Panneau de controle", 1500, 500);

    cv::createTrackbar("Altitude                  ", "Panneau de controle", &altitude_computed, ALTITUDE_RANGE, updateSliders);
    cv::createTrackbar("Taux de montee       ", "Panneau de controle", &ascentRate_computed, ASCENTRATE_RANGE, updateSliders);
    cv::createTrackbar("Angle d'attaque      ", "Panneau de controle", &angle_computed, ANGLE_RANGE, updateSliders);
    cv::createTrackbar("Puissance", "Panneau de controle", &power_computed, POWER_RANGE, updateSliders);

    cv::setMouseCallback("Panneau de controle", onMouse);
   

}
std::string formatDecimal(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}
// Switch case pour l'affichage textuel de l'état avionique
std::string enumToString(avionicState state) {
    switch (state) {
        case AU_SOL: return "AU_SOL";
        case CHANGEMENT_ALT: return "CHANGEMENT_ALT";
        case VOL_CROISIERE: return "VOL_CROISIERE";
        default: return "Unknown";
    }
}

void UI_Process(cv::Mat& image) {
    image = cv::Mat::zeros(800, 1000, CV_8UC3);

    // Affichage textuel des entrées de l'utilisateur sur l'UI
    std::string altitude_text = "Altitude: " + std::to_string(altitude_UI) + " pieds";
    std::string taux_text = "Taux de montee: " + formatDecimal(ascentRate_UI) + " m/min";
    std::string angle_text = "Angle d'attaque: " + formatDecimal(angle_UI) + " deg.";
    std::string power_text = "Puissance: " + std::to_string(power_UI) + " %";
    std::string autopilot_ON = "Autopilot ON";
    std::string autopilot_OFF = "Autopilot OFF";

    // Affichage textuel des données actuelles de l'avion sur l'UI
    std::string currentState_text = enumToString(currentState);
    std::string system_altitude_text = "Altitude: " + std::to_string(system_altitude) + " pieds";
    std::string system_climbRate_text = "Taux de montee: " + formatDecimal(system_climbRate) + " m/min";
    std::string system_power_text = "Puissance: " + std::to_string(system_power) + " %";

    // Zone d'affichage textuelle des commandes 
    cv::Rect rect(10, 30, 990, 250);
    cv::Scalar background_color(50, 50, 50);
    cv::Scalar text_color(255, 255, 255);
    cv::rectangle(image, rect, background_color, -1);
    cv::putText(image, "Commande", cv::Point(20, 25), cv::FONT_HERSHEY_DUPLEX, 1.0, text_color, 2);

    // Zone d'affichage textuelle des valeurs actuelle de l'avion (reçues du calculateur)
    cv::Rect rect2(10, 480, 430, 250); 
    cv::rectangle(image, rect2, background_color, -1); 
    cv::putText(image, "Valeurs actuelles", cv::Point(20, 470), cv::FONT_HERSHEY_DUPLEX, 1.0, text_color, 2);
    cv::putText(image, system_altitude_text, cv::Point(20, 530), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
    cv::putText(image, system_climbRate_text, cv::Point(20, 590), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
    cv::putText(image, system_power_text, cv::Point(20, 660), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);


    // Zone d'affichage des messages d'avertissement contextuels
    cv::Rect bordure_jaune(450, 480, 550, 250);
    cv::Scalar jaune(58, 245, 235);
    cv::rectangle(image, bordure_jaune, jaune, -1);
    cv::Rect centre_noir(450 + 5, 480 + 5, 550 - 10, 250 - 10);
    cv::Scalar noir(0, 0, 0);
    cv::rectangle(image, centre_noir, noir, -1);
    cv::putText(image, "Avertissements ", cv::Point(460, 470), cv::FONT_HERSHEY_DUPLEX, 1.0, text_color, 2);

  
    cv::Scalar rouge(0, 0, 255);
    cv::Scalar blanc(0, 0, 255);
    // Avertissement pour l'angle au décolage
    if(!takeoffAngle_flag){
        cv::putText(image, "ANGLE INVALIDE: ", cv::Point(465, 530), cv::FONT_HERSHEY_SIMPLEX, 0.8, rouge, 2);
        cv::putText(image, "Il faut un angle positif pour decoller", cv::Point(465, 555), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
    }
    // Avertissement pour le décrochage (15 degrés)
    if(angle_UI > 14.9 && !autopilot){
        cv::putText(image, "RISQUE DE DECROCHAGE", cv::Point(465, 530), cv::FONT_HERSHEY_SIMPLEX, 0.8, rouge, 2);
        cv::putText(image, "Diminuez l'angle", cv::Point(465, 555), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
    }
    // Avertissements de puissance
    if(power_problems_flag && currentState != AU_SOL){
        if(autopilot){
            cv::putText(image, "VITESSE MAX ATTEINTE: ", cv::Point(465, 530), cv::FONT_HERSHEY_SIMPLEX, 0.8, rouge, 2);
            cv::putText(image, "Vous pouvez diminuer la puissance", cv::Point(465, 555), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
        }
        else{
            cv::putText(image, "PUISSANCE INSUFFISANTE: ", cv::Point(465, 530), cv::FONT_HERSHEY_SIMPLEX, 0.8, rouge, 2);
            cv::putText(image, "Augmentez l'angle ou diminuez la vitesse ", cv::Point(465, 555), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
        }
        
    }



    
    
    // Écran d'affichage de l'état avionique
    cv::ellipse(image, cv::Point(780, 385), cv::Size(200, 60), 0, 0, 360, cv::Scalar(135, 206, 235), -1);  
    cv::putText(image, "Etat avionique", cv::Point(660, 315), cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(255, 255, 255), 2);  
    cv::putText(image, currentState_text, cv::Point(615, 395), cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(0, 0, 0), 1);




    // Sélection des paramètres pour le décolage
    if (currentState == AU_SOL){
        cv::putText(image, altitude_text, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
        cv::putText(image, taux_text, cv::Point(20, 120), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
        cv::putText(image, angle_text, cv::Point(20, 190), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
        cv::putText(image, power_text, cv::Point(20, 260), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);


    }
    // Logique d'affichage des valeurs de commandes
    else {
        if(autopilot){
            
                cv::putText(image, altitude_text, cv::Point(20, 120), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
                cv::putText(image, power_text, cv::Point(20, 190), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);

        }
        else {

                cv::putText(image, taux_text, cv::Point(20, 120), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
                cv::putText(image, angle_text, cv::Point(20, 190), cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 1);
                
            

        }
    }

 

    // Dessiner les boutons pour le contrôle de précision
    cv::rectangle(image, cv::Point(600, 100), cv::Point(700, 130), cv::Scalar(0, 255, 0), -1); // Incrémenter
    cv::rectangle(image, cv::Point(700, 100), cv::Point(800, 130), cv::Scalar(0, 0, 255), -1); // Décrémenter
    cv::putText(image, "+", cv::Point(600, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, "-", cv::Point(700, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
  
    cv::rectangle(image, cv::Point(600, 170), cv::Point(700, 200), cv::Scalar(0, 255, 0), -1); // Incrémenter
    cv::rectangle(image, cv::Point(700, 170), cv::Point(800, 200), cv::Scalar(0, 0, 255), -1); // Décrémenter
    cv::putText(image, "+", cv::Point(600, 195), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, "-", cv::Point(700, 195), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    // Ajout des boutons pour des bonds de 10  
    std::string step_text_P = "+ 1";
    std::string step_text_M = "- 1";

    if (autopilot) {
        step_text_P = "+ 10";
        step_text_M = "- 10";
    }
    cv::rectangle(image, cv::Point(800, 100), cv::Point(900, 130), cv::Scalar(0, 100, 0), -1); // Bond +10
    cv::rectangle(image, cv::Point(900, 100), cv::Point(1000, 130), cv::Scalar(0, 0, 139), -1); // Bond -10
    cv::putText(image, step_text_P, cv::Point(800, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, step_text_M, cv::Point(900, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    
    cv::rectangle(image, cv::Point(800, 170), cv::Point(900, 200), cv::Scalar(0, 100, 0), -1); // Bond +10
    cv::rectangle(image, cv::Point(900, 170), cv::Point(1000, 200), cv::Scalar(0, 0, 139), -1); // Bond -10
    cv::putText(image, step_text_P, cv::Point(800, 195), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, step_text_M, cv::Point(900, 195), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
      
  // too much magic numbers....
    // Affiche de tous les boutons lorsqu'au sol
    if (currentState == AU_SOL) {

        // Dessiner les boutons pour l'altitude
        cv::rectangle(image, cv::Point(600, 30), cv::Point(700, 60), cv::Scalar(0, 255, 0), -1); // Incrémenter
        cv::rectangle(image, cv::Point(700, 30), cv::Point(800, 60), cv::Scalar(0, 0, 255), -1); // Décrémenter
        cv::putText(image, "+", cv::Point(600, 55), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        cv::putText(image, "-", cv::Point(700, 55), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

        // Bonds de 10 pour l'altitude
        cv::rectangle(image, cv::Point(800, 30), cv::Point(900, 60), cv::Scalar(0, 100, 0), -1); // Bond +10
        cv::rectangle(image, cv::Point(900, 30), cv::Point(1000, 60), cv::Scalar(0, 0, 139), -1); // Bond -10
        cv::putText(image, "+ 10", cv::Point(800, 55), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        cv::putText(image, "- 10", cv::Point(900, 55), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

        // Dessiner les boutons pour la puissance
        cv::rectangle(image, cv::Point(600, 240), cv::Point(700, 270), cv::Scalar(0, 255, 0), -1); // Incrémenter
        cv::rectangle(image, cv::Point(700, 240), cv::Point(800, 270), cv::Scalar(0, 0, 255), -1); // Décrémenter
        cv::putText(image, "+", cv::Point(600, 265), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        cv::putText(image, "-", cv::Point(700, 265), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

        // Bonds de 10 puissance
        cv::rectangle(image, cv::Point(800, 240), cv::Point(900, 270), cv::Scalar(0, 100, 0), -1); // Bond +10
        cv::rectangle(image, cv::Point(900, 240), cv::Point(1000, 270), cv::Scalar(0, 0, 139), -1); // Bond -10
        cv::putText(image, "+ 10", cv::Point(800, 265), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        cv::putText(image, "- 10", cv::Point(900, 265), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);



    }
      // Bouton ON/OFF de l'autopilote
    if (currentState != AU_SOL){
        if(autopilot){
            cv::rectangle(image, cv::Point(20, 305), cv::Point(360, 360), cv::Scalar(0, 165, 255), -1); // Orange
            cv::putText(image, autopilot_ON, cv::Point(65, 342), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        }
        else{
            cv::rectangle(image, cv::Point(20, 305), cv::Point(360, 360), cv::Scalar(255, 165, 0), -1); // bleu
            cv::putText(image, autopilot_OFF, cv::Point(65, 342), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        }
    }


}

// Fonction de callback pour détecter les clics de souris
void onMouse(int event, int x, int y, int, void*) {
    // Vérifier si un clic a eu lieu dans les zones des boutons
    if (event == cv::EVENT_LBUTTONDOWN) {
        if(currentState == AU_SOL){ 
        
            // Altitude
            if (x >= 600 && x <= 700 && y >= 30 && y <= 60) {
                altitude_computed = std::min(ALTITUDE_RANGE, altitude_computed + 1);
                cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);
            }
            if (x >= 700 && x <= 800 && y >= 30 && y <= 60) {
                altitude_computed = std::max(0, altitude_computed - 1);
                cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);
            }
            if (x >= 800 && x <= 900 && y >= 30 && y <= 60) { // +10
                altitude_computed = std::min(ALTITUDE_RANGE, altitude_computed + 10);
                cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);
            }
            if (x >= 900 && x <= 1000 && y >= 30 && y <= 60) { // -10
                altitude_computed = std::max(0, altitude_computed - 10);
                cv::setTrackbarPos("Altitude                  ","Panneau de controle", altitude_computed);
            }
            // Puissance
            if (x >= 600 && x <= 700 && y >= 240 && y <= 270) {
                power_computed = std::min(POWER_RANGE, power_computed + 1);
                cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);
            }
            if (x >= 700 && x <= 800 && y >= 240 && y <= 270) {
                power_computed = std::max(0, power_computed - 1);
                cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);
            }
            if (x >= 800 && x <= 900 && y >= 240 && y <= 270) { // +10
                power_computed = std::min(POWER_RANGE, power_computed + 10);
                cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);
            }
            if (x >= 900 && x <= 1000 && y >= 240 && y <= 270) { // -10
                power_computed = std::max(0, power_computed- 10);
                cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);
            }
            // Taux de montée
            if (x >= 600 && x <= 700 && y >= 100 && y <= 130) {
                ascentRate_computed = std::min(ASCENTRATE_RANGE, ascentRate_computed + 1);
                cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
            }
            if (x >= 700 && x <= 800 && y >= 100 && y <= 130) {
                ascentRate_computed = std::max(0, ascentRate_computed - 1);
                cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
            }
            if (x >= 800 && x <= 900 && y >= 100 && y <= 130) { // +10
                ascentRate_computed = std::min(ASCENTRATE_RANGE, ascentRate_computed + 10);
                cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
            }
            if (x >= 900 && x <= 1000 && y >= 100 && y <= 130) { // -10
                    ascentRate_computed = std::max(0, ascentRate_computed - 10);
                    cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
            }

            // Angle d'attaque
            if (x >= 600 && x <= 700 && y >= 170 && y <= 200) {
                angle_computed = std::min(ANGLE_RANGE, angle_computed + 1);
                cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);
            }
            if (x >= 700 && x <= 800 && y >= 170 && y <= 200) {
                angle_computed = std::max(0, angle_computed - 1);
                cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);
            }
            if (x >= 800 && x <= 900 && y >= 170 && y <= 200) { // +10
                angle_computed = std::min(ANGLE_RANGE, angle_computed + 10);
                cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);
            }
            if (x >= 900 && x <= 1000 && y >= 170 && y <= 200) { // -10
                angle_computed = std::max(0, angle_computed - 10);
                cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);
            }
        }
        else {
            
            // Bouton de l'autopilote
            if (x >= 20 && x <= 360 && y >= 305 && y <= 360) { // ON/OFF
                autopilot = !autopilot;
            }
          
      
            // Taux de montée ou Altitude
            if (x >= 600 && x <= 700 && y >= 100 && y <= 130) {
                if (autopilot){
                    altitude_computed = std::min(ALTITUDE_RANGE, altitude_computed + 1);
                    cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);
                }
                else {
                    ascentRate_computed = std::min(ASCENTRATE_RANGE, ascentRate_computed + 1);
                    cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
                }
            }
            if (x >= 700 && x <= 800 && y >= 100 && y <= 130) {
                if(autopilot) {
                    altitude_computed = std::max(0, altitude_computed - 1);
                    cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);
                }
                else {
                    ascentRate_computed = std::max(0, ascentRate_computed - 1);
                    cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
                }
            }
            if (x >= 800 && x <= 900 && y >= 100 && y <= 130) { // +10
                    if(autopilot) {
                        altitude_computed = std::min(ALTITUDE_RANGE, altitude_computed + 10);
                        cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);

                    }
                    else {
                        ascentRate_computed = std::min(ASCENTRATE_RANGE, ascentRate_computed + 10);
                        cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
                    }
            }
            if (x >= 900 && x <= 1000 && y >= 100 && y <= 130) { // -10
                    if(autopilot) {
                        altitude_computed = std::max(0, altitude_computed - 10);
                        cv::setTrackbarPos("Altitude                  ", "Panneau de controle", altitude_computed);
                    }
                    else{
                        ascentRate_computed = std::max(0, ascentRate_computed - 10);
                        cv::setTrackbarPos("Taux de montee       ", "Panneau de controle", ascentRate_computed);
                    }
                   
            }

            // Angle d'attaque ou puissance
            if (x >= 600 && x <= 700 && y >= 170 && y <= 200) {
                if(autopilot){
                    power_computed = std::min(POWER_RANGE, power_computed + 1);
                    cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);

                }
                
                else {
                    angle_computed = std::min(ANGLE_RANGE, angle_computed + 1);
                    cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);
                }
      
            }
            if (x >= 700 && x <= 800 && y >= 170 && y <= 200) {
                if(autopilot){
                    power_computed = std::max(0, power_computed - 1);
                    cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);

                }
                else {
                    angle_computed = std::max(0, angle_computed - 1);
                    cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);
                }
              
            }
            if (x >= 800 && x <= 900 && y >= 170 && y <= 200) { // +10
                if(autopilot){
                    power_computed = std::min(POWER_RANGE, power_computed + 10);
                    cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);

                }
                else {
                    angle_computed = std::min(ANGLE_RANGE, angle_computed + 10);
                    cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);

                }
           
            }
            if (x >= 900 && x <= 1000 && y >= 170 && y <= 200) { // -10
                if(autopilot){
                    power_computed = std::max(0, power_computed- 10);
                    cv::setTrackbarPos("Puissance", "Panneau de controle", power_computed);

                }              
                else {
                    angle_computed = std::max(0, angle_computed - 10);
                    cv::setTrackbarPos("Angle d'attaque      ", "Panneau de controle", angle_computed);

                }
            
            }
        }


        }
        

    }
    

// Met à jour la description textuelle des commandes sur l'UI
void updateSliders(int, void*) {
    
    altitude_UI = altitude_computed;
    ascentRate_UI = ascentRate_computed / 10.0;
    angle_UI = (angle_computed - 160.0) / 10.0;
    power_UI = power_computed;

}
