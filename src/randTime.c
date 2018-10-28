#include "randTime.h"

//call this ONCE! sets up the random timer.
void initRandTime(){
	clock_t start = clock();
	srand(start);
}

//returns a random time between 0 and 1 second
float randomTimeSecond(){
	float randomTime=(float)rand()/RAND_MAX;
	return randomTime;
}
