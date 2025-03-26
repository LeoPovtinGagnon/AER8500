#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include "client.hpp"

// Constantes pour la connexion TCP
std::string server_ip = "127.0.0.1";
int server_port = 8080;

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

// Variable pour arrêter le thread de réception
bool receiving = true;
int received_value = 0;

// Fonction pour recevoir les données dans un thread séparé
void receiveData(int client_socket) {
    while (receiving) {
        int new_value = receiveInt(client_socket);
        if (new_value != -1) {
            received_value = new_value;
            std::cout << "Valeur reçue du serveur: " << received_value << std::endl;
        }
    }
}

void updateSliders(int, void*) {
    // Comparaison des anciennes et nouvelles valeurs
    bool altitudeModified = (altitude_UI != altitude_slider);
    const float TOLERANCE = 0.01f;
    bool ascentRateModified = std::abs(ascentRate_UI - ascentRate_slider / 10.0) > TOLERANCE;
    bool angleModified = std::abs(angle_UI - (angle_slider - 160.0) / 10.0) > TOLERANCE;

    altitude_UI = altitude_slider;
    ascentRate_UI = ascentRate_slider / 10.0;
    angle_UI = (angle_slider - 160.0) / 10.0;

    if (altitudeModified) {
        isAltitudeModified = true;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
    if (ascentRateModified) {
        isAscentRateModified = true;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
    if (angleModified) {
        isAngleModified = true;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
}

void ValueUpdate() {
    using namespace std::chrono;
    auto currentTime = steady_clock::now();
    auto duration = duration_cast<milliseconds>(currentTime - lastUpdateTime);

    if (duration.count() >= DELAY_TIME) {
        if (isAltitudeModified) {
            std::cout << "Envoi du message pour l'altitude : " << altitude_UI << " pieds" << std::endl;
            isAltitudeModified = false;
        }
        if (isAscentRateModified) {
            std::cout << "Envoi du message pour le taux de montée : " << ascentRate_UI << " m/min" << std::endl;
            isAscentRateModified = false;
        }
        if (isAngleModified) {
            std::cout << "Envoi du message pour l'angle d'attaque : " << angle_UI << " deg." << std::endl;
            isAngleModified = false;
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


// Fonction pour dessiner les boutons
void drawButtons(cv::Mat& image) {
    // Dessiner les boutons pour l'altitude
    cv::rectangle(image, cv::Point(600, 30), cv::Point(700, 60), cv::Scalar(0, 255, 0), -1); // Incrémenter
    cv::rectangle(image, cv::Point(700, 30), cv::Point(800, 60), cv::Scalar(0, 0, 255), -1); // Décrémenter
    cv::putText(image, "+", cv::Point(650, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, "-", cv::Point(750, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    // Dessiner les boutons pour le taux de montée
    cv::rectangle(image, cv::Point(600, 100), cv::Point(700, 130), cv::Scalar(0, 255, 0), -1); // Incrémenter
    cv::rectangle(image, cv::Point(700, 100), cv::Point(800, 130), cv::Scalar(0, 0, 255), -1); // Décrémenter
    cv::putText(image, "+", cv::Point(650, 120), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, "-", cv::Point(750, 120), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    // Dessiner les boutons pour l'angle d'attaque
    cv::rectangle(image, cv::Point(600, 170), cv::Point(700, 200), cv::Scalar(0, 255, 0), -1); // Incrémenter
    cv::rectangle(image, cv::Point(700, 170), cv::Point(800, 200), cv::Scalar(0, 0, 255), -1); // Décrémenter
    cv::putText(image, "+", cv::Point(650, 190), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(image, "-", cv::Point(750, 190), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
}

// Fonction de callback pour détecter les clics de souris
void onMouse(int event, int x, int y, int, void*) {
    // Vérifier si un clic a eu lieu dans les zones des boutons
    if (event == cv::EVENT_LBUTTONDOWN) {
        // Altitude
        if (x >= 600 && x <= 700 && y >= 30 && y <= 60) {
            altitude_slider = std::min(ALTITUDE_RANGE, altitude_slider + 1);
            cv::setTrackbarPos("Altitude", "Panneau de controle", altitude_slider);
        }
        if (x >= 700 && x <= 800 && y >= 30 && y <= 60) {
            altitude_slider = std::max(0, altitude_slider - 1);
            cv::setTrackbarPos("Altitude", "Panneau de controle", altitude_slider);
        }

        // Taux de montée
        if (x >= 600 && x <= 700 && y >= 100 && y <= 130) {
            ascentRate_slider = std::min(ASCENTRATE_RANGE, ascentRate_slider + 1);
            cv::setTrackbarPos("Taux de montee", "Panneau de controle", ascentRate_slider);
        }
        if (x >= 700 && x <= 800 && y >= 100 && y <= 130) {
            ascentRate_slider = std::max(0, ascentRate_slider - 1);
            cv::setTrackbarPos("Taux de montee", "Panneau de controle", ascentRate_slider);
        }

        // Angle d'attaque
        if (x >= 600 && x <= 700 && y >= 170 && y <= 200) {
            angle_slider = std::min(ANGLE_RANGE, angle_slider + 1);
            cv::setTrackbarPos("Angle d'attaque", "Panneau de controle", angle_slider);
        }
        if (x >= 700 && x <= 800 && y >= 170 && y <= 200) {
            angle_slider = std::max(0, angle_slider - 1);
            cv::setTrackbarPos("Angle d'attaque", "Panneau de controle", angle_slider);
        }
    }
}

int main() {
    int client_socket = start_client(server_ip, server_port);
    std::thread receiveThread(receiveData, client_socket);

    cv::namedWindow("Panneau de controle", cv::WINDOW_NORMAL);
    cv::resizeWindow("Panneau de controle", 1200, 300);

    cv::createTrackbar("Altitude", "Panneau de controle", &altitude_slider, ALTITUDE_RANGE, updateSliders);
    cv::createTrackbar("Taux de montee", "Panneau de controle", &ascentRate_slider, ASCENTRATE_RANGE, updateSliders);
    cv::createTrackbar("Angle d'attaque", "Panneau de controle", &angle_slider, ANGLE_RANGE, updateSliders);

    cv::setMouseCallback("Panneau de controle", onMouse);

    while (true) {
        // Mise à jour des sliders et de l'UI
        cv::Mat image;
        UI_Process(image);
        drawButtons(image);
        cv::imshow("Panneau de controle", image);

        // Envoi des messages si nécessaire
        ValueUpdate();

        // Sortir si l'utilisateur ferme la fenêtre
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    receiving = false;
    receiveThread.join();
    cv::destroyAllWindows();
    return 0;
}
