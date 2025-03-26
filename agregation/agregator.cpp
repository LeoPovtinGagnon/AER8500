#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

// Variables pour les sliders de l'UI
int altitude_slider = 0;
int ascentRate_slider = 0;
int angle_slider = 160;

// Valeurs textuelles des commandes sur le UI
int altitude_UI = 0;
float ascentRate_UI = 0;
float angle_UI = 0;

// Valeurs pour les trackbar du UI
const int ASCENTRATE_RANGE = 8000;
const int ALTITUDE_RANGE = 40000;
const int ANGLE_RANGE = 320;

const int DELAY_TIME = 1000;  // Temps d'attente de 1 seconde avant d'envoyer un changement 

// Flags pour savoir si une valeur a été modifiée
bool isAltitudeModified = false;
bool isAscentRateModified = false;
bool isAngleModified = false;

// Dernière fois qu'un message a été envoyé
std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();

void updateSliders(int, void*) {
    // Comparaison des anciennes et nouvelles valeurs
    bool altitudeModified = (altitude_UI != altitude_slider);
    // Une petite tolérance pour éviter des erreurs dues aux comparaisons de floats
    const float TOLERANCE = 0.01f;
    bool ascentRateModified = std::abs(ascentRate_UI - ascentRate_slider / 10.0) > TOLERANCE;
    bool angleModified = std::abs(angle_UI - (angle_slider - 160.0) / 10.0) > TOLERANCE;

    // Mettre à jour les valeurs
    altitude_UI = altitude_slider;
    ascentRate_UI = ascentRate_slider / 10.0;
    angle_UI = (angle_slider - 160.0) / 10.0;

    // Si une valeur a changé, marquer la modification
    if (altitudeModified) {
        isAltitudeModified = true;
        lastUpdateTime = std::chrono::steady_clock::now();  // Réinitialiser le timer dès qu'une modification est détectée
    }
    if (ascentRateModified) {
        isAscentRateModified = true;
        lastUpdateTime = std::chrono::steady_clock::now();  // Réinitialiser le timer dès qu'une modification est détectée
    }
    if (angleModified) {
        isAngleModified = true;
        lastUpdateTime = std::chrono::steady_clock::now();  // Réinitialiser le timer dès qu'une modification est détectée
    }
}

void ValueUpdate() {
    using namespace std::chrono;
    auto currentTime = steady_clock::now();
    auto duration = duration_cast<milliseconds>(currentTime - lastUpdateTime);

    // Si le temps écoulé est supérieur à 1 seconde et qu'une modification a eu lieu, envoie le message
    if (duration.count() >= DELAY_TIME) {
        if (isAltitudeModified) {
            std::cout << "Envoi du message pour l'altitude : " << altitude_UI << " pieds" << std::endl;
            isAltitudeModified = false;  // Réinitialiser le flag après envoi
        }
        if (isAscentRateModified) {
            std::cout << "Envoi du message pour le taux de montée : " << ascentRate_UI << " m/min" << std::endl;
            isAscentRateModified = false;  // Réinitialiser le flag après envoi
        }
        if (isAngleModified) {
            std::cout << "Envoi du message pour l'angle d'attaque : " << angle_UI << " deg." << std::endl;
            isAngleModified = false;  // Réinitialiser le flag après envoi
        }
    }
}

std::string formatDecimal(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

void UI_Process(cv::Mat& image) {
    image = cv::Mat::zeros(250, 1000, CV_8UC3);

    std::string altitude_text = "Altitude: " + std::to_string(altitude_UI) + " pieds";
    std::string taux_text = "Taux de montee: " + formatDecimal(ascentRate_UI) + " m/min";
    std::string angle_text = "Angle d'attaque: " + formatDecimal(angle_UI) + " deg.";

    cv::Rect rect(10, 30, 500, 180);
    cv::Scalar background_color(50, 50, 50);
    cv::Scalar text_color(255, 255, 255);
    cv::rectangle(image, rect, background_color, -1);

    cv::putText(image, "Commande", cv::Point(20, 25), cv::FONT_HERSHEY_DUPLEX, 1.0, text_color, 2);
    cv::putText(image, altitude_text, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 0.7, text_color, 1);
    cv::putText(image, taux_text, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 0.7, text_color, 1);
    cv::putText(image, angle_text, cv::Point(20, 140), cv::FONT_HERSHEY_SIMPLEX, 0.7, text_color, 1);
}

void adjustValue(char key) {
    switch (key) {
        case 'w': 
            altitude_slider = std::min(ALTITUDE_RANGE, altitude_slider + 1);
            cv::setTrackbarPos("Altitude                 ", "Panneau de controle", altitude_slider);
            break;
        case 's': 
            altitude_slider = std::max(0, altitude_slider - 1);
            cv::setTrackbarPos("Altitude                 ", "Panneau de controle", altitude_slider);
            break;
        case 'e': 
            ascentRate_slider = std::min(ASCENTRATE_RANGE, ascentRate_slider + 1);
            cv::setTrackbarPos("Taux de montee          ", "Panneau de controle", ascentRate_slider);
            break;
        case 'd': 
            ascentRate_slider = std::max(0, ascentRate_slider - 1);
            cv::setTrackbarPos("Taux de montee          ", "Panneau de controle", ascentRate_slider);
            break;
        case 'r': 
            angle_slider = std::min(ANGLE_RANGE, angle_slider + 1);
            cv::setTrackbarPos("Angle d'attaque          ", "Panneau de controle", angle_slider);
            break;
        case 'f': 
            angle_slider = std::max(0, angle_slider - 1);
            cv::setTrackbarPos("Angle d'attaque          ", "Panneau de controle", angle_slider);
            break;
    }
}


int main() {
    cv::namedWindow("Panneau de controle", cv::WINDOW_NORMAL);
    cv::resizeWindow("Panneau de controle", 1200, 300);

    cv::createTrackbar("Altitude                 ", "Panneau de controle", &altitude_slider, ALTITUDE_RANGE, updateSliders);
    cv::createTrackbar("Taux de montee          ", "Panneau de controle", &ascentRate_slider, ASCENTRATE_RANGE, updateSliders);
    cv::createTrackbar("Angle d'attaque          ", "Panneau de controle", &angle_slider, ANGLE_RANGE, updateSliders);

    int i = 0;
    while (true) {
        cv::Mat image;
        UI_Process(image);
        cv::imshow("Panneau de controle", image);
        char key = (char)cv::waitKey(1);
        if (key == 27) break; // Échap pour quitter
        if (key != -1) adjustValue(key);

        // Vérifier les mises à jour à chaque itération
        ValueUpdate();
    }

    cv::destroyAllWindows();
    return 0;
}
