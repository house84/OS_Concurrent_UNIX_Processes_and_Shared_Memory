/*
 * Author: Nick House
 * Projec: Concurrent UNIX Processes and Shared Memory
 * Course: CS-4760 Operating Systems, Spring 2021
 * File Name: bin_adder.c
 */ 

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


//===Func Prototypes===//

static void process(); 								//Solution 4
static void critical_section();				//Critical Part to avoid race condition 
static void openLogFile(); 						//Open Logfile
static void closeLogFile(); 					//Close Logfile
static struct sharedMemory *shmptr; 	//Global shared memory pointer


//===Global Variables===//

int xx; 															//Child Process Index, will be mod for array addition algo 
int yy; 															//Depth for array addition algo 
int indexNum; 												//Index for of flag[] 
time_t curtime; 											//Initalize Value for Time
FILE * filepointer; 									//Logfile Pointer


//=== main function for child Process bin_adder()===//

int main(int argc, char * argv[], char * envp[]) {
	
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

	return EXIT_SUCCESS;
}



//===Multi Process Solution 4===//

static void process ( const int i ) {

		int j = NULL; 												//Local to each process
		int n = 20; 													//Number of Processes
		
		//Because there may be more operations
		//than indices in flag check to 
		//Ensure Index to be assigned is not 
		//being used by previous Process.

		while( shmptr->flag[i] != idle ){

			wait(NULL); 		
		}	
	
		do{
	
			shmptr->flag[i] = want_in; 					//raise my flag
			j = shmptr->turn; 									//set local variable
				
			while ( j != i ){

				j = ( shmptr->flag[j] != idle ) ? shmptr->turn : ( j + 1 ) % n; 

			}
			
			//Declare intention to enter critical section
			shmptr->flag[i] = in_cs; 

			//Check that no one else is in critical section 
			for( j = 0; j < n; j++ ){

				if( ( j != i ) && ( shmptr->flag[j] == in_cs )){

					break; 

				}
			}
			
		} while (( j < n ) || ( shmptr->turn != i && shmptr->flag[shmptr->turn] != idle )); 
	 

	//Assign turn to self and enter critical section 
	shmptr->turn = i; 
 
	
	//Call Critical Section Function
	critical_section();


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

static void critical_section(){

	//Get value to get index for second value
	int myExp = shmptr->depth - yy; 
	int offset = (int)pow(2, myExp); 
	int secValue = xx + offset; 

	//Local Variable for pid
	pid_t mypid = getpid(); 

	//Get time
	time(&curtime);

	//Print Critcal Section Message
	fprintf(stderr, "Critical Section Entered\nTime: %sPID: %ld\n", ctime(&curtime), mypid); 

	//Open logfile
	openLogFile(); 

	//wait 1 second before writing
	sleep(1); 

	//===Algo Explanation to Perform Addition of indices===//
	//
	//xx = Index of first integer to be summed in dataArr
	//shmptr->depth = Max Depth of Array when created dynamically
	//yy = current depth of xx passed to Child from parent
	//offset is the amount of indices to add to xx to get second index
	//offset = 2 ^ ( shmptr->depth/yy)
	//SecValue is the index of the second integer to be summed 
	//secValue = xx + offset
	
	//Set sum to index xx
	shmptr->dataArr[xx] += shmptr->dataArr[offset];  

	//Get time
	time(&curtime); 

	//stderr Print to console
	//fprintf( stderr, "Time: %sPID: %d    Index: %d    Depth: %d    Sum: %d\n", ctime(&curtime), mypid, xx, yy, shmptr->dataArr[xx]); 
	
	//Print to logfile
	fprintf( filepointer, "\nTime: %sPID: %d    Index: %d    Depth: %d    Sum: %d\n", ctime(&curtime), mypid, xx, yy, shmptr->dataArr[xx]); 

	//Close file
	closeLogFile();

	//Exit Message
	fprintf(stderr, "Critical Section Exited\nTime: %sPID: %ld\n\n", ctime(&curtime), mypid); 

}


//Open/Create LogFile

static void openLogFile(){

	filepointer = fopen("logfile", "a+"); 

	//Error Checking
	if( filepointer == NULL ){

		perror("bin_adder: ERROR: Failed to open File "); 
		exit(EXIT_FAILURE); 
	} 
}

//Free File Pointer

static void closeLogFile(){

	fclose(filepointer); 

}

