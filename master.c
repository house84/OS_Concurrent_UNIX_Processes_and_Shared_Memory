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
#include "master.h"

void help(); 			//Usage Info
void bin_adder();		//Spawn Children
int validateData();		//Validate Data from file 
void addData(); 		//Extract Data to Shared Memory
void setTimer(); 		//Set Timer
int getTimer();   		//Get Timer
struct itimerval timer; 	//Set Global Timer Struct
struct sharedMemory *shmptr; 	//Global Pointer Shared Memory

int lines = 0; 			//Length of Datafile
//int depth = 0; 			//Depth of Array
//int leaves = 0; 		//Number of leaves for array
int time = 100; 		//Default Time Out
int children = 20; 		//Default Children

int main(int argc, char * argv[]) {

	//Testing stdout Program Name
	printf("Master\n"); 
	
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
				  printf("Children: %d\n", children); 
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
	
	//IPC_CREAT = create new segment
	//S_IRUSR = allow attach in read mode
	//S_IWUSR = allow attach in write mode	
	int shmid = shmget(key, sizeof(struct sharedMemory), IPC_CREAT | S_IRUSR | S_IWUSR); 
	
//Remove	struct sharedMemory *shmptr; 

	//Check for error shmget()
	if( shmid == -1 ){
		perror("Error: shmget \n");
		exit(EXIT_FAILURE);
	}

	//attatch memory segmet with id
	shmptr = (struct sharedMemory *) shmat(shmid, NULL, 0);

	
	//===Parse DataFile for Input and Validate===//
	lines = validateData(argv[file_index]); 
	addData(argv[file_index]); 


	//===Fork Child Processes===// 

	int i; 
	//Testing Fork 
	for( i = 0; i < children; ++i ){
		
//Remove	printf("Testing Fork Loop: %d\n", i); 

		//Testing Fork
		bin_adder(i, getTimer(), shmid);
		
		//Test Timer
	//	sleep(1);  
	} 

	//Allow Children to Terminate
	while( wait(NULL) > 0 ); 
		

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
	printf("%s [-h] [-s x] [-t time] datafile\n", program); 
	printf("\n%s -i x\t\t-> argument required for this option: x indicates the number of children allowed to exist (default 20)\n\n", program);  
	printf("%s -t time\t-> argument required for this option: time in seconds after the process will terminate even if not finished.\n\n", program);
	printf("%s datafile\t-> datafile required, this file must only contain integers, and only one integer per page. No blank lines.\n\n", program); 

}


//===Fork Child Processes===//

void bin_adder(int n, int time, int myshmid){

	pid_t process_id = fork();
	
//	fprintf(stderr, "In forkit\n"); 	
	
	//Check for error
	if( process_id == -1 ) {
		
		perror("Error: Fork \n");
		exit(EXIT_FAILURE);  
	}
	if( process_id == 0 ) {
	
		//Create string in buffer 
		char buffer_1[10];
		sprintf(buffer_1, "%d", n); 
		
		char buffer_2[10];
		sprintf(buffer_2, "%d", time); 

		char buffer_3[50];
		sprintf(buffer_3, "%d", myshmid); 
		
		printf("In else\n"); 

		//execl branch execl() takes null terminated strings as args
		
		
		//execl("./branch", "branch", buffer, time, myshmid, (char *) NULL); 
		if( execl("./branch", "branch", buffer_1, buffer_2, buffer_3, (char *) NULL) == -1) {
			
			perror("ERROR execl\n");
		} 
	
		//Exit Child Process
		exit(EXIT_SUCCESS); 
	}
}


//===Set Timer===//

void setTimer(int time){
	
//Remove	struct itimerval timer;
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


//===Get Time===//

int getTimer(){
	
	int currTime = getitimer(ITIMER_REAL, &timer); 	
	
//Remove  	//This is for testing
//	printf("Timer: %d\n", timer.it_value.tv_sec); 

	return timer.it_value.tv_sec;
}


//===Validate and Copy DataFile===//

int validateData(char * filename){
	
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
		
		
		//Store Data (dataValue) into Array of shared memory

//		printf("Line: %d = %d\n", mylines, dataValue); 
	}


	//===Close File and Free Memory===//
	
	fclose(filepointer); 
	
	if(data){ free(data); }

	return mylines; 
}


//===Allocate Space and Add to Shared Memory===//

void addData(char * filename ){
	
//	printf("In addData - File: %s\n", filename);

	//===Set datanumber Array to appropriate Size===//
	//calculate size of array
	shmptr->depth = ceil(log2(lines));
	shmptr->leaves = pow(2, shmptr->depth);  
	int arrSize = shmptr->depth+1; 
	//Test Expected outputs
//	printf("Depth = %d\tLeaves = %d\n",shmptr->depth, shmptr->leaves);  
		
	shmptr->datanumber = malloc(arrSize*shmptr->leaves*sizeof(int)); 	

/*	
 *	Rules for 2D Dynamic -> int *arr = malloc(rows*columns*sizeof(datatype));
 * 	To access use -> arr[i*columns + j] in place of arr[i][j]
 */	
	
	int i, j; 
	
	//set Values to zero
	for(i = 0; i <= shmptr->depth; ++i ){
		for(j = 0; j < shmptr->leaves; ++j){
			shmptr->datanumber[i*shmptr->leaves + j] = 0; 
		}
	}	
	
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

		//Test What will be added to array
//		printf("Iteration = %d\tData = %d\n", k, dataValue);

		shmptr->datanumber[shmptr->depth*shmptr->leaves + k] = dataValue; 

		//increment
		++k;  
	}

		
	//===Close Files and Free Memory===//
	
	fclose(filepointer); 
	
	if(data){ free(data); }
		

} 
