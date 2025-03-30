#include <opencv2/opencv.hpp>

// Initialise le UI
void UI_init();

// Fonction gérant les éléments à afficher sur le UI
void UI_Process(cv::Mat& image);

// Callback des clicks de sourie
void onMouse(int event, int x, int y, int, void*);
// Update les valeurs textuelles
void updateSliders(int, void*);

// Valeurs maximales possibles des entrées
extern const int ASCENTRATE_RANGE ;
extern const int ALTITUDE_RANGE ;
extern const int ANGLE_RANGE ;
extern const int POWER_RANGE ;

// Entrées de l'utilisateur
extern int altitude_computed;
extern int ascentRate_computed;
extern int angle_computed;
extern int power_computed;

