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
#include "master.h"

static void help(); 			//Usage Info
static void spawn_child();		//Spawn Children
static int validateData();		//Validate Data from file 
static void addData(); 			//Extract Data to Shared Memory
static void setTimer(); 		//Set Timer
static int getTimer();   		//Get Timer
static void signalHandler(); 		//Signal Handler for termination of program
static struct itimerval timer; 		//Set Global Timer Struct
static struct sharedMemory *shmptr; 	//Global Pointer Shared Memory

int lines = 0; 				//Length of Datafile n
int time = 100; 			//Default Time Out
int children = 20; 			//Default Max amount of Concurrent Children
int totalProc = 0; 			//Set Total Processes as n/2
int shmid = NULL; 			//Global shmid
pid_t  *pidArray; 			//Array for child pid
bool flag = false; 			//Flag to check Child Processes and Signal handling
bool sigFlag = false; 			//Flag to check if Signal has been called to terminate processes


int main(int argc, char * argv[]) {

	//Testing stdout Program Name
	printf("Master\n"); 
	
	//initiate Signal Handling
	signal(SIGINT, signalHandler); //CTRL+C

	int c = 0; 

	//Handle Inputs with getopt()
	while((c = getopt(argc, argv, "hi:t:")) != -1) {
	
		switch(c) {
			
			case 'h': help(argv[0]);
				  if(argc == 2) { exit(EXIT_SUCCESS);}
				  break;  
		
			case 'i': //printf("%s option i\n", optarg); 
				  children = atoi(optarg); 
				  //Test Setting
				  //printf("Children: %d\n", children); 
				  break;
	
			case 't': //printf("%s option t\n", optarg); 
				  time = atoi(optarg); 
				  //Test Setting
				  //printf("Time: %d\n", time); 
				  break; 

			case ':': printf("Argument required\n"); 
				  exit(EXIT_FAILURE); 
				  break; 
			
			default:  //Invalid Args Exit  
				  exit(EXIT_FAILURE); 
				 
		}
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
		
		printf("ERROR: Datafile required.\n"); //optind: %d\targc: %d\n", optind, argc); }
		exit(EXIT_FAILURE);
	} 	
	
	
	//==Start Timer===//
	setTimer(time); 

	//===Allocate Shared Memory===//	
	//generate unique key
	key_t key = ftok("makefile", 'a'); 
	
	//Set shared memory id	
	shmid = shmget(key, sizeof(struct sharedMemory), IPC_CREAT | S_IRUSR | S_IWUSR);  

	//Check for error shmget()
	if( shmid == -1 ){
		perror("Error: shmget \n");
		exit(EXIT_FAILURE);
	}

	//attatch memory segmet with id
	shmptr = (struct sharedMemory *) shmat(shmid, NULL, 0);

	
	//===Parse DataFile for Input and Validate===//
	
	//Validate DataFile format
	lines = validateData(argv[file_index]); 
	shmptr->dataInputs = lines; 
	
	//Add Data to Array
	addData(argv[file_index]);	

	//Set total required Processes = N-1
	if( lines % 2 == 0 ){
		totalProc = lines -1; 
	}else{
		totalProc = lines; 
	} 

	
	//Allocate Memory for pidArray
	pidArray = (int *) malloc(totalProc*sizeof(int));  


	//===Fork Child Processes===// 
	// Create Child Processes not to exceed children max limit. 
	// Outter Loop handle levels of the tree
	// inner Loop will process pairs for computation.  

	//totalProc -> total processes needed
	//lines -> integer inputs in array
	//currProc -> Amount of Processes Running
	//summedProc -> Amount of processes run
	
//	int currProc = 0; 
	int summedProc = 0; 
	bool complete = false; 
	
	while( summedProc < totalProc && complete == false ){
		
		//Total All Proc
		++summedProc;
	
		//Track concurrent procedures
		++shmptr->currProc; 

		//If all Procedures Assigned to Children Break
		if(summedProc == totalProc) { 
			complete = true;
			break; 
  		}  
		
		//Limit concurrent Child Processes to 20
		while(shmptr->currProc == 20) { 
			
			wait(NULL); 
		}

		//Slow for testing
		//sleep(1); 

		//Spawn Child Process
		spawn_child((summedProc-1), getTimer(), shmid); 
	
			
	}
	


	//Allow Children to Terminate
	while( wait(NULL) > 0 ){} //printf("Wait\n"); } 
	
	
	//Print Array
	int z; 
 
	for( z = 0 ; z < shmptr->leaves; ++z){
		
		printf("Loop: "); 
		printf(" %d ", shmptr->dataArr[z]); 
	}
	
	printf("\n"); 

	

	//===Detatch Shared Memory===//	

	//Detatch memory 
	shmdt( shmptr ); 

	//Destroy shared memory
	shmctl(shmid, IPC_RMID, NULL);

	//Kill Child Processes
//	int i; 
//	for(i = 0; i < totalProc; ++i){
		
//		kill(pidArray[i], SIGKILL); 
//	}

	return EXIT_SUCCESS; 
}


//===Usage page -h Option===//
 
void help(char * program){

	printf("\n%s Usage Page\n", program);
	printf("\n%s -h\n", program);
	printf("%s [-h] [-s x] [-t time] datafile\n", program); 
	printf("\n%s -i x\t\t-> argument required for this option: x indicates the number of children allowed to exist (default 20)\n\n", program);  
	printf("%s -t time\t-> argument required for this option: time in seconds after the process will terminate even if not finished.\n\n", program);
	printf("%s datafile\t-> datafile required, this file must only contain integers, and only one integer per page. No blank lines.\n\n", program); 

}


//===Fork Child Processes===//

static void spawn_child(int n, int time, int myshmid){

	
	//Check for Signal Flag
	if( sigFlag == true ){  return; }	
	
	pid_t process_id; 

	//Create New Child Process
	if((process_id = fork()) < 0 ){ 
		
		perror("Error: Fork \n");
		exit(EXIT_FAILURE);  
	}

	else if( process_id == 0 ){
			
		//Block Signal Handler from Terminating
		flag = true; 		
	
		//Add child id to array
		pidArray[n-1] = process_id;  

		//Return Flag to false				
		flag = false; 


		//Create string in buffer 
		char buffer_1[10];
		sprintf(buffer_1, "%d", n); 
		
		char buffer_2[10];
		sprintf(buffer_2, "%d", time); 

		char buffer_3[50];
		sprintf(buffer_3, "%d", myshmid); 

		//Add Logic to Call with xx and yy: 

		//Call bin_adder w/execl()
		if( execl("./bin_adder", "bin_adder", buffer_1, buffer_2, buffer_3, (char *) NULL) == -1) {
			
			perror("ERROR execl\n");
		} 
	
		//Exit Child Process
		exit(EXIT_SUCCESS); 
	}
}


//===Set Timer===//

static void setTimer(int time) {

	signal(SIGALRM, signalHandler); 
	
	timer.it_value.tv_sec = time;
	timer.it_value.tv_usec = 0; 
	timer.it_interval.tv_sec = 0; 
 	timer.it_interval.tv_usec = 0; 

	//Set timer and perror Check
	if( setitimer( ITIMER_REAL, &timer, NULL) == -1 ){
		
		perror("Error: Timer"); 
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
	while(flag == true){
	//	sleep(1);
	}
	
	//Allow child processes to end
	while(wait(NULL) > 0); 

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
		
		perror("Error whle opening file "); 
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
			fprintf(stderr, "Error: Datafile Contains Non-Integer\n"); 
			exit(EXIT_FAILURE); 
		}
		if (endptr == myString) {
			
			fprintf(stderr, "Error: Datafile Contains Non-Integer\n"); 
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

	//Create 1D array allocate memory for datanumber
	shmptr->dataArr = (int *) malloc((arrLength)* sizeof(int));  

	//===Add Validated file to Memory===// 
	
	//Open File
	FILE * filepointer;
        filepointer = fopen(filename, "r");

	//Error Check 
	if( filepointer == NULL) {

                perror("Error whle opening file ");
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

	
	//Test Array Allocated

//	int i; 	
//	for(i = 0; i < arrLength; ++i){
//	
//		printf("Value: %d\n", shmptr->dataArr[i]); 
//	}
		
	//===Close Files and Free Memory===//
	
	fclose(filepointer); 
	
	if(data){ free(data); }
} 
