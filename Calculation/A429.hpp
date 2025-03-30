// Variables de l'agrégateur les plus récentes
extern uint32_t desired_altitude;
extern uint32_t desired_power;
extern float desired_angle;
extern float desired_climbRate;
void receiveARINC429Message(int sock);
void sendARINC429Message(int label, uint32_t data, int sock);
uint32_t encodeARINC429Message(int label, int value);


