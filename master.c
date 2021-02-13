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
#include "master.h"

void help();
void forkit();  

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
	
			case 't': printf("%s option t\n", optarg); 
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

	//Open File
	char ch, filename[50];
	FILE *filepointer; 
	filepointer = fopen(argv[file_index], "r"); 	
	
	//Validate File Location
	if(filepointer == NULL){

		perror("Error while opening file ");
		exit(EXIT_FAILURE); 
	}

	//Validation Variables
	int characters = 1;
        int lines = 1;
        int flag_1 = 0;
	
	//===Parse through file contents===//
	
	while((ch = fgetc(filepointer)) != EOF ){
		
	//	printf("%c", ch); 
		characters++; 	

		//Input Validation
		if ( flag_1 == 0 && isdigit((unsigned char) ch)){
			
			flag_1 = 1; 
		}
		else if( flag_1 == 0 && !isdigit((unsigned char) ch )){
			
			if(ch == '\n'){ printf("\nERROR: Line: %d Index: %d -> File must contain one integer per line\n", lines, characters, ch);}

			else { printf("\nERROR: Line: %d Index: %d Character: %c -> File must contain only integers and only one integer per line\n", lines, characters, ch);} 
			
			exit (EXIT_FAILURE); 
		}
		else if( flag_1 == 1 && !isdigit((unsigned char) ch)){
		
			flag_1 = 2; 
		}
		else if( flag_1 == 2 && ch != '\n' && ch != '\0'){// && ch != ' ' ){

			printf("\nERROR: Line: %d Index: %d Character: %c ->  Only one integer allowed per line\n", lines, characters, ch); 
			
			exit (EXIT_FAILURE); 
		}


		//Check New Line
		if( ch == '\n' ){
			
			lines++;
		//	printf("New Line -> Flag: %d\n", flag_1); 
			characters = 0;
			flag_1 = 0; 
		} 		
	}



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



	//===Fork Child Processes===// 

	int i; 
	//Testing Fork 
	for( i = 0; i < children; ++i ){
		
	//	printf("Testing Fork Loop: %d\n", i); 

		//Testing Fork
		forkit(i);
	} 

	//Allow Children to Terminate
	while( wait(NULL) > 0 ); 
		

	//===Detatch Shared Memory===//	

	//Detatch memory 
	shmdt( shmptr ); 

	//Destroy shared memory
	shmctl(shmid, IPC_RMID, NULL);


	//Close datafile
	fclose(filepointer);


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

void forkit(int n){

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
		execl("./branch", "Branch",  buffer, (char *) NULL); 

		//Exit Child Process
		exit(EXIT_SUCCESS); 
	}
}

	 
