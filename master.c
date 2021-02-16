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
#include "master.h"

void help(); 		//Usage Info
void bin_adder();	//Spawn Children
void getData();		//Extract Data from file
void setTimer(); 	//Set Timer
int getTimer();   	//Get Timer
struct itimerval timer; //Set Global Timer Struct


int main(int argc, char * argv[]) {

	//Testing stdout Program Name
	printf("Master\n"); 
	
	//Set Variables and Defaults
	int c = 0; 
	int time = 100; 
	int children = 20; 

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
	
	//IPC_CREAT = create new segment
	//S_IRUSR = allow attach in read mode
	//S_IWUSR = allow attach in write mode	
	int shmid = shmget(key, sizeof(struct sharedMemory), IPC_CREAT | S_IRUSR | S_IWUSR); 
	
	struct sharedMemory *shmptr; 

	//Check for error shmget()
	if( shmid == -1 ){
		perror("Error: shmget \n");
		exit(EXIT_FAILURE);
	}

	//attatch memory segmet with id
	shmptr = (struct sharedMemory *) shmat(shmid, NULL, 0);

	
	//===Parse DataFile for Input===//
	getData(argv[file_index]); 



	//===Fork Child Processes===// 

	int i; 
	//Testing Fork 
	for( i = 0; i < children; ++i ){
		
	//	printf("Testing Fork Loop: %d\n", i); 

		//Testing Fork
		bin_adder(i, getTimer());
		
		//Test Timer
		sleep(1);  
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

void bin_adder(int n, int time){

	pid_t process_id = fork();
	
//	fprintf(stderr, "In forkit\n"); 	
	
	//Check for error
	if( process_id == -1 ) {
		
		perror("Error: Fork \n");
		exit(EXIT_FAILURE);  
	}
	else if( process_id == 0 ) {
	
		//Create string in buffer 
		char buffer[10];
		sprintf(buffer, "%d", n); 

		//execl branch
		execl("./branch", "Branch",  buffer, time, (char *) NULL); 

		//Exit Child Process
		exit(EXIT_SUCCESS); 
	}
}


//===Set Timer===//
//use setitimer()//

void setTimer(int time){
	
//	struct itimerval timer;
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
//Use getitimer()//
int getTimer(){
	
	int currTime = getitimer(ITIMER_REAL, &timer); 	
	
	printf("Timer: %d\n", timer.it_value.tv_sec); 

	return timer.it_value.tv_sec;
}


//===Validate and Copy DataFile===//
void getData(char * filename){
	
	//Get File Location
	FILE * filepointer;
	filepointer = fopen(filename, "r"); 	

	//Validate File Location
	if( filepointer == NULL) {
		
		perror("Error whle opening file "); 
		exit(EXIT_FAILURE); 
	}

	
	//Get file Content
	int lines = 0;
	int dataValue;
	long value;  
	char *myString, *endptr;   
	char *data = NULL; 
	size_t len = 50; 

	while(getline(&data,&len, filepointer) != -1 ){
		
		lines++; 
  	
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

		printf("Line: %d = %d\n", lines, dataValue); 
	}

	
	


	//===Close File and Free Memory===//
	
	fclose(filepointer); 
	
	if(data){ free(data); }
} 
