#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#define MESSAGE_SIZE_MAX 255
#define TRANSMISSION_SIZE_MAX (MESSAGE_SIZE_MAX+7) // +7 for the packet information

#define PREAMBLE_INDEX 0
#define VERSION_INDEX 1
#define SOURCE_INDEX 2
#define DESTINATION_INDEX 3
#define LENGTH_INDEX 4
#define CRC_FLAG_INDEX 5
#define MESSAGE_INDEX 6
#define MINIMUM_MANCHESTER_LENGTH (8*7)

#define MY_SOURCE_ADDRESS 11

#define PREAMBLE 0x55
#define VERSION 0x01

#define BACKOFF_LIMIT (15)

struct packet
{
	char Sync;
	char Version;
	char Source;
	char Destination;
	char Length;
	char CRCflag;
	char CRCtrailer;
};

void transmitter_init();
void startTransmission(uint8_t destination, uint8_t CRCflag, char *array, int amountOfChars);
void stopTransmission();
void handleAnyTransmissionCollision();

#endif /* CHANNEL_H_ */
