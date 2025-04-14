#include <cmath>
void receiveARINC429Message(int sock);
void sendARINC429Message(int label, uint32_t data, int sock);
uint32_t encodeARINC429Message(int label, int value);




