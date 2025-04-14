#include <cmath>
#include <mutex>
// Gestionnaire d'état
void stateManager();
// Gestionnaire autopilote
void autopilotManager();
// Gestionnaire en mode pilotage manuel
void manualManager();

// Définir une énumération pour la machine à états
enum  avionicState {
    AU_SOL,    // L'avion est au sol
    CHANGEMENT_ALT, // L'avion change d'altitude
    VOL_CROISIERE,  // Croisière atteinte
    LANDING // État interne pour l'atterissage (sera affiché comme AU_SOL)
};

// Variable de l'état avionique
extern avionicState currentState;
extern bool autopilot;
// Flag qui détecte quand la puissance est constante
extern bool stagnantPowerFlag;
// Flag qui détecte quand une manoeuvre demande plus de puissance que le moteur peut en donner
extern bool notEnoughPowerFlag;
// Détection d'une puissance inutilement élevée (taux de montée max atteint)
extern bool tooMuchPowerFlag;
// Flag vrai quand l'altitude doit diminuer
extern bool descendingFlag;
// Variables actuelles de l'avion
extern int live_altitude;
extern float live_climbRate;
extern float live_angle;
extern int live_power;

// Variables de l'agrégateur les plus récentes
extern uint32_t desired_altitude;
extern uint32_t desired_power;
extern float desired_angle;
extern float desired_climbRate;

// Bool à modifier pour choisir quel protocole recoit les données (redondance)
extern bool protocolSelector;

