#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>
#include "shared.h"

void process(); 		//Solution 4
void critical_section();		//Critical Part to avoid race condition 
struct sharedMemory *shmptr; 	//Global shared memory pointer
int xx; 
int yy; 

int main(int argc, char * argv[], char * envp[]) {

//Testing Output using Fork and execl 

	printf("Child -> %s: %s Time: %s shmid: %s\n", argv[0], argv[1], argv[2], argv[3]);  
	
	//Set shmptr from shmid
	shmptr = (struct sharedMemory *) shmat(atoi(argv[3]), NULL, 0);  
	
	//argv[1] xx
	xx = atoi(argv[1]); 

	//argv[2] yy
	yy = atoi(argv[2]); 

	//Call Solution 4
	process(xx); 
	
	return EXIT_SUCCESS;
}




//===Multi Process Solution 4===//
void process ( const int i ) {

		int j; 		//Local to each process
		int n; 		//Number of Processes Total
		n = 19; 		

		do {
	
			shmptr->flag[i] = want_in; 	//raise my flag
			j = shmptr->turn; 			//set local variable
				
			while ( j != i ) 
				j = ( shmptr->flag[j] != idle ) ? shmptr->turn : ( j + 1 ) % n; 

			//Declare intention to enter critical section

			shmptr->flag[i] = in_cs; 

			//Check that no one else is in critical section 

			for( j = 0; j < n; j++ )
				if( ( j != i ) && ( shmptr->flag[j] == in_cs ))
					break; 
			
		} while ( j < n || ( shmptr->turn != i && shmptr->flag[shmptr->turn] != idle )); 
	 
		
	//Assign turn to self and enter critical section 

	shmptr->turn = i; 
	
	//Enter Critical
	fprintf(stderr, "Enter Critical %d\n", i); 
	
	critical_section(); 

	//Exit Section
	fprintf(stderr, "Exit Critical %d\n", i); 

			
	j = ( shmptr->turn + 1 ) % n; 

	while ( shmptr->flag[j] == idle); 
		j = ( j + 1 ) % n; 

	//Assign turn to next waiting process; change flag to idle

	shmptr->turn = j; 
	shmptr->flag[i] = idle; 
	 
}

		

	
//Exicute Algo
void critical_section(){


	int i;   
	for(i = 0; i < shmptr->leaves; ++i){
		
		shmptr->dataArr[i] += 1; 

		printf("[%d] ", shmptr->dataArr[i] ); 
	}
	printf("\n"); 	



	shmptr->currProc--;
}
