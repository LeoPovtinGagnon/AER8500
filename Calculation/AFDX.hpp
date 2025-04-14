void receiveAFDXMessage(int sock);
void sendAFDXMessage(int sock);
// Variables de l'agrégateur les plus récentes
extern uint32_t AFDX_altitude;
extern uint32_t AFDX_power;
extern float AFDX_angle;
extern float AFDX_climbRate;