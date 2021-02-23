#ifndef SHARED_H
#define SHARED_H

#include <math.h>


enum state {idle, want_in, in_cs};  

struct sharedMemory {
  
	enum state flag[20]; 
	int turn; 
	int dataInputs;  
	int depth;
	int leaves;    
	int dataArr[];  

};


#endif
