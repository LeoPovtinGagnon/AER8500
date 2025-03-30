// Définir une énumération pour la machine à états
enum  avionicState {
    AU_SOL,    // L'avion est au sol
    CHANGEMENT_ALT, // L'avion change d'altitude
    VOL_CROISIERE   // Croisière atteinte
};
// Variable de l'état avionique
extern avionicState currentState;
extern bool autopilot;
// Variables internes (envoyées par le calculateur)
extern int system_altitude;
extern float system_climbRate;
extern int system_power;
