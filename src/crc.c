#include "crc.h"

static char CRC_TABLE[256];

//this will generate the CRC table for out polynomial 0x107 as defined in the lab doc. Called once to create the table.
void generate_CRC_Table(){
	uint16_t polynomial=0x107;//this ix x^8+x^2+x+1=0x107
	for(int d=0;d<256;d++){
		char thisByte=(char)d;
		for(int bit=0;bit<8;bit++){
			if((thisByte&0x80)!=0){
				thisByte<<=1;
				thisByte^=polynomial;
			}else{
				thisByte<<=1;
			}
		}
		CRC_TABLE[d]=thisByte;
	}
}

//Called when data is received to verify crc or data is being transferred to send a crc
char calculate_CRC(char *inputArray,int numberOfChars){
	char r_CRC=0x0;
	for(int i=0 ; i<numberOfChars ; i++){
		unsigned char inputCRC=(char)(inputArray[i]^r_CRC);
		r_CRC=(char)CRC_TABLE[inputCRC];
	}
	return r_CRC;
}

