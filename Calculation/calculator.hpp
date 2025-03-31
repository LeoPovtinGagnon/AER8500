// Définir une énumération pour la machine à états
enum  avionicState {
    AU_SOL,    // L'avion est au sol
    CHANGEMENT_ALT, // L'avion change d'altitude
    VOL_CROISIERE   // Croisière atteinte
};

// Variable de l'état avionique
extern avionicState currentState;
extern bool autopilot;
// Flag qui détecte quand la puissance est constante
extern bool stagnantPowerFlag;