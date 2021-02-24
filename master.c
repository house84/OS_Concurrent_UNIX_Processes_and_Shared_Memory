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
#include <math.h> 
#include <stdbool.h> 
#include <signal.h>
#include "shared.h"

static void help(); 									//Usage Info
static void spawn();									//Spawn Children
static int validateData();						//Validate Data from file 
static void addData(); 								//Extract Data to Shared Memory
static void setTimer(); 							//Set Timer
static int getTimer();   							//Get Timer
static void signalHandler(); 					//Signal Handler for termination of program
static struct itimerval timer; 				//Set Global Timer Struct
static struct sharedMemory *shmptr; 	//Global Pointer Shared Memory

int lines = 0; 												//Length of Datafile n
int time = 100; 											//Default Time Out
int children = 20; 										//Default Max amount of Concurrent Children
int totalProc = 0;										//Set Total Processes as n-1
int xx = 0; 													//Index Number xx
int yy = 0; 													//Depth yy
int pidCount = 0; 										//Track child PID Count
int shmid = NULL;											//Global shmid
pid_t  *pidArray; 										//Array for child pid
bool flag = false; 										//Flag to check Child Processes and Signal handling
bool sigFlag = false; 								//Flag to check if Signal has been called to terminate processes 

int main(int argc, char * argv[]) {
	
	//initiate Signal Handling
	signal(SIGINT, signalHandler); 

	int c = 0; 

	//Handle Inputs with getopt()
	while((c = getopt(argc, argv, "hs:t:")) != -1) {
	
		switch(c) {
			
			//Usage Argument 
			case 'h': 
					
					help(argv[0]);
				  if(argc == 2) { exit(EXIT_SUCCESS);}
				  break;  
			
			//Number of Concurrent Child Process Arg
			case 's': 

				  children = atoi(optarg); 
				  break;
			
			//Set Timer Value 
			case 't':

				  time = atoi(optarg);  
				  break; 
			
			//Pass Message When Arg Required
			case ':': 
					
					fprintf(stderr, "Argument required\n"); 
				  exit(EXIT_FAILURE); 
				  break; 
			
			//Invalid Arg Message
			default:  

					fprintf(stderr, "Invalid Argument, see help page\n"); 
				  exit(EXIT_FAILURE); 
				 
		}
	}
	


	//Check for 0 Value inputs for Children or Time and Exit
	if(children == 0 || time == 0 ) {
		
		exit(EXIT_SUCCESS); 

	}


	//Set Index for Filename
	int file_index; 

	if(argc > 1){

		file_index = optind; 
	}
	else {
 
		file_index = 1; 
	}

	
	//Check for DataFile Argument 
	if( optind == argc ) { 
		
		fprintf(stderr, "%s: ERROR: Datafile required.\n", argv[0]);  //optind: %d\targc: %d\n", optind, argc); }
		exit(EXIT_FAILURE);
	} 	
	

	//Validate datafile content
	lines = validateData(argv[file_index]);
	
	//==Start Timer===//
	setTimer(time); 

	
	//===Allocate Shared Memory===//	
	
	//generate unique key
	key_t key = ftok("Makefile", 'a');

	//Create Dynamic Memory Size dependent on datafile Size
	size_t memSize = sizeof(struct sharedMemory) + (lines*sizeof(int)); // + (lines*sizeof(enum state)); 

	//Set Allocate shared memory for shmid	
	shmid = shmget(key, sizeof(memSize), IPC_CREAT | S_IRUSR | S_IWUSR);  

	//Check for error shmget()
	if( shmid == -1 ){
		perror("Master: Error: shmget ");
		exit(EXIT_FAILURE);
	} 
	
	//attatch memory segmet with id
	shmptr = (struct sharedMemory *) shmat(shmid, NULL, 0);

	
	//===Parse DataFile for Input and Validate===//
	
	//Validate DataFile format
	shmptr->dataInputs = lines; 
	
	//Add Data to Array
	addData(argv[file_index]);	

	//Set total required Processes = n-1
	totalProc = lines - 1; 
	
	//Allocate Memory for pidArray (Not Needed)
	pidArray = (int *) malloc(totalProc*sizeof(int));  


	//=== Spawn Child Processes ===//
	
	int summedProc = 0;
	int level = 1; 

	//Set Initial Depth
	yy = shmptr->depth; 	

	//Spawn Initial Group of Processes 
	while( summedProc < children){

		//Pass proper index (xx) to child based on level 
		if( xx % (int)(pow(2, level)) == 0 ){
		
			//Total All Proc 
			++summedProc;	
		
			//Call Initial Child Processes **Remove global shmid from call
			spawn( xx, yy, shmid); 
		}

		//Increment xx
		++xx; 

		//Check if Depth (yy) needs to be decramented
		if(( xx % (int)(pow(yy, 2)/2)) == totalProc){
			
			--yy; 
			++level;

		}
	}
	
	//Spawn Remaining Processes One at a Time Until Finished
	while( summedProc <= totalProc ){

			//Wait for One Process to end
			wait(NULL); 
		
			//Check for proper index to pass to child
			if( xx % (int)(pow(2, level)) == 0 ){

				//Increment Count to Next Process
				++summedProc;
		
				//Call Next Child Process
				spawn( xx, yy, shmid); 

			}

			//Increment xx
			++xx; 

			//Check if depth needs to be decramented
			if(( xx % (int)(pow(yy, 2)/2)) == totalProc){

				--yy;
				++level; 

			}
	}


	//=== Allow Processes to Finish ===//
	
	//Allow Children to Terminate
	while( wait(NULL) > 0){ } 


	//===Detatch Shared Memory===//	

	//Detatch memory 
	shmdt( shmptr ); 

	//Destroy shared memory
	shmctl(shmid, IPC_RMID, NULL);


	return EXIT_SUCCESS; 
}


//===Usage page -h Option===//
 
void help(char * program){

	printf("\n%s Usage Page\n", program);
	printf("\n%s -h\n", program);
	printf("%s [-h] [-s i] [-t time] datafile\n", program); 
	printf("\n%s -s i\t\t-> argument required for this option: x indicates the number of children allowed to exist (default 20)\n\n", program);  
	printf("%s -t time\t-> argument required for this option: time in seconds after the process will terminate even if not finished.\n\n", program);
	printf("%s datafile\t-> datafile required, this file must only contain integers, and only one integer per page. No blank lines.\n\n", program); 

}


//===Fork Child Processes===//

static void spawn(int x, int y, int myshmid){

	
	//Check for Signal Flag
	if( sigFlag == true ){  return; }	

	//initialize Variable
	pid_t process_id; 

	//Create New Child Process
	if((process_id = fork()) < 0 ){ 
		
		perror("Master: Error: Fork \n");
		exit(EXIT_FAILURE);  
	}

	//Ensure Child Process
	else if( process_id == 0 ){
			
		//Block Signal Handler from Terminating
		flag = true; 		
	
		//Add child id to array
		pidArray[pidCount] = process_id;  
		++pidCount; 

		//Return Flag to false				
		flag = false; 

		//Create string in buffer 
		char buffer_x[10];
		sprintf(buffer_x, "%d", x); 
		
		char buffer_y[10];
		sprintf(buffer_y, "%d", y); 

		char buffer_shm[50];
		sprintf(buffer_shm, "%d", myshmid); 

		//Add Logic to Call with xx and yy: 

		//Call bin_adder w/execl()
		if( execl("./bin_adder", "bin_adder", buffer_x, buffer_y, buffer_shm, (char *) NULL) == -1) {
			
			perror("Master: ERROR: execl() failure ");
		} 
	
		//Exit Child Process
		exit(EXIT_SUCCESS); 
	}
}



//===Set Timer===//

static void setTimer(int mytime) {

	signal(SIGALRM, signalHandler); 
	
	timer.it_value.tv_sec = mytime;
	timer.it_value.tv_usec = 0; 
	timer.it_interval.tv_sec = 0; 
 	timer.it_interval.tv_usec = 0; 

	//Set timer and perror Check
	if( setitimer( ITIMER_REAL, &timer, NULL) == -1 ){
		
		perror("Master: Error: Timer failure "); 
		exit(EXIT_FAILURE); 
	}	
}	


//===Get Time Passed===//

static int getTimer(){
	
	int currTime = getitimer(ITIMER_REAL, &timer); 	
	
	return time-timer.it_value.tv_sec;
}


//===Signal Handler===//
static void signalHandler(int sig){

	sigFlag = true; 
	
	if( sig == SIGINT ){
	
		fprintf(stderr,"\nProgram Terminated by User\n");
	}
	else{
		
		fprintf(stderr, "\nProgram Terminateed due to Time\n");
	}
 	
	//Allow Current PID to finish
	while(flag == true){}
	
	//Allow child processes to end
	//while(wait(NULL) > 0); 

	//===Detatch and Delete Shared Memory===//
	shmdt( shmptr );
	shmctl(shmid, IPC_RMID, NULL); 
		
	//===Kill Child Processes===//
	int i; 
	for(i = 0; i < totalProc; ++i){
	
		kill(pidArray[i], SIGKILL); 
	}
	
	exit(EXIT_SUCCESS); 
}


//===Validate and Copy DataFile===//
static int validateData(char * filename){
	
	//Get File Location
	FILE * filepointer;
	filepointer = fopen(filename, "r"); 	

	//Validate File Location
	if( filepointer == NULL) {
		
		perror("Master: Error: Failed to open file "); 
		exit(EXIT_FAILURE); 
	}

	
	//Get file Content
	int mylines = 0;
	int dataValue;
	long value;  
	char *myString, *endptr;   
	char *data = NULL; 
	size_t len = 50; 
	
	
	while(getline(&data,&len, filepointer) != -1 ){
		
		mylines++; 
  	
		myString = data; 	 
		dataValue = atoi(data);
		value = strtol(myString, &endptr, 10); 
		
		//Validate Input
		if(( errno == ERANGE && ( value == LONG_MAX || value == LONG_MIN)) || (errno != 0 && value == 0 )) {
			fprintf(stderr, "Maseter: Error: Datafile Contains Non-Integer\n"); 
			exit(EXIT_FAILURE); 
		}
		if (endptr == myString) {
			
			fprintf(stderr, "Master: Error: Datafile Contains Non-Integer\n"); 
			exit(EXIT_FAILURE); 
		} 

	}


	//===Close File and Free Memory===//
	
	fclose(filepointer); 
	
	if(data){ free(data); }

	return mylines; 
}


//===Allocate Space and Add to Shared Memory===//

static void addData(char * filename ){
	
	//===Set datanumber Array to appropriate Size===//
	
	//calculate Depth and Length of array
	shmptr->depth = ceil(log2(lines));
	shmptr->leaves = pow(2, shmptr->depth);  
	
	int arrLength = shmptr->leaves+1;

	//===Add Validated file to Memory===// 
	
	//Open File
	FILE * filepointer;
  filepointer = fopen(filename, "r");

	//Error Check 
	if( filepointer == NULL) {

    perror("Master: Error: Failed to open file ");
  	exit(EXIT_FAILURE);
  
	}


	//Variables for extracting data
	char *data = NULL;
	size_t len = 50; 
	int dataValue = 0;
	int k = 0;  
	
	//Exctract int
	while(getline(&data,&len, filepointer) != -1 ){
	
		dataValue = atoi(data);

		//Add Data to Array
		shmptr->dataArr[k] = dataValue; 
	
		//increment
		++k;  
	}
		
	//===Close Files and Free Memory===//
	
	fclose(filepointer); 
	
	if(data){ 
		
		free(data); 
	}
} 
