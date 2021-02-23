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
#include <time.h> 
#include "shared.h"

void process(); 							//Solution 4
void critical_section();			//Critical Part to avoid race condition 
void openLogFile(); 					//Open Logfile
void closeLogFile(); 					//Close Logfile
struct sharedMemory *shmptr; 	//Global shared memory pointer
int xx; 											//Child Process Index, will be mod for array addition algo 
int yy; 											//Depth for array addition algo 
int indexNum; 								//Index for of flag[] 
time_t curtime; 							//Initalize Value for Time
FILE * filepointer; 					//Logfile Pointer

int main(int argc, char * argv[], char * envp[]) {

//Testing Output using Fork and execl 

	printf("Child -> %s: %s Time: %s shmid: %s\n", argv[0], argv[1], argv[2], argv[3]);  
	
	//Set shmptr from shmid
	shmptr = (struct sharedMemory *) shmat(atoi(argv[3]), NULL, 0);  
	
	//argv[1] indexNum%20 
	indexNum = atoi(argv[1])%20;  
	
	//argv[1] xx
	xx = atoi(argv[1]); 

	//argv[2] yy
	yy = atoi(argv[2]); 

	//Call Solution 4
	process(indexNum); 
	
	//release Memory Ptr
	shmdt(shmptr); 
	
	//test
	fprintf(stderr, "Child Process Exit\n"); 

	return EXIT_SUCCESS;
}




//===Multi Process Solution 4===//
void process ( const int i ) {

		int j = NULL; 												//Local to each process
		int n = 20; 													//Number of Processes
	
		fprintf(stderr, "i: %d\n", i); 

		//Ensure Current Index is not 
		//being used by previous Process
		while( shmptr->flag[i] != idle ){

			wait(NULL); 		
		}	
	
		do{

			fprintf(stderr, "In Do, J: %d Turn: %d\n", j, shmptr->turn); 
	
			shmptr->flag[i] = want_in; 		//raise my flag
			j = shmptr->turn; 						//set local variable
				
			while ( j != i ){

				j = ( shmptr->flag[j] != idle ) ? shmptr->turn : ( j + 1 ) % n; 

			}

			fprintf(stderr, "Post while j != i\n"); 
			
			//Declare intention to enter critical section
			shmptr->flag[i] = in_cs; 

			//Check that no one else is in critical section 
			for( j = 0; j < n; j++ ){

				if( ( j != i ) && ( shmptr->flag[j] == in_cs )){

					break; 

				}
			}
			
		} while (( j < n ) || (( shmptr->turn != i && shmptr->flag[shmptr->turn] != idle ))); 
	 
		
	//Assign turn to self and enter critical section 
	shmptr->turn = i; 
	
	//Enter Critical
	fprintf(stderr, "Enter Critical %d\n", i); 
	
	critical_section(); 

	//Exit Section
	fprintf(stderr, "Exit Critical %d\n", i); 

	//Index to next Array
	j = ( shmptr->turn + 1 ) % n; 

	while ( shmptr->flag[j] == idle){

		j = ( j + 1 ) % n; 
	}

	//Assign turn to next waiting process; change flag to idle

	shmptr->turn = j; 
	shmptr->flag[i] = idle; 
}

	
//Execute Algo in Critical Section
void critical_section(){

	time(&curtime); 
	
	printf("CRITICAL SECTION\n"); 


	openLogFile(); 

	//Test Logfile Output
	fprintf(stderr, "Index: %d  Depth: %d  Time: %s  PID: %ld\n", xx, yy, ctime(&curtime), getpid()); 

	closeLogFile(); 

}

//	int i;   
//	for(i = 0; i < shmptr->leaves; ++i){
		
//		shmptr->dataArr[i] += 1; 

//		printf("[%d] ", shmptr->dataArr[i] ); 
//	}
//	printf("\n"); 	
 
//}

//Open/Create LogFile
void openLogFile(){

	filepointer = fopen("logfile", "a+"); 

	//Error Checking
	if( filepointer == NULL ){

		perror("bin_adder: ERROR: Failed to open File "); 
		exit(EXIT_FAILURE); 
	} 
}

//Free File Pointer
void closeLogFile(){

	fclose(filepointer); 

}

