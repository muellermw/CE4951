#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#define TRANSMISSION_SIZE_MAX 255

void transmitter_init();
void startTransmission(char *array, int amountOfChars);
void stopTransmission();

#endif /* CHANNEL_H_ */

