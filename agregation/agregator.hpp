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
extern float system_angle;
// Flag pour gérer l'attérissage
extern bool landing_flag;

// Flag pour les avertissements sur l'interface liés à la puissance 
extern bool power_problems_flag;

// Flag pour veiller à ce que l'angle d'attaque soit positif au décolage
extern bool takeoffAngle_flag;


