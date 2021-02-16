#ifndef MASTER_H
#define MASTER_H

#include <math.h>

enum state {idle, want_in, in_cs}; 

struct sharedMemory {

	enum state flags[20]; 
	enum state turn; 
	int depth;
	int leaves; 
	int *datanumber;  
};


#endif
