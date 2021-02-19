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


int main(int argc, char * argv[], char * envp[]) {

//Testing Output using Fork and execl 

	printf("Child -> %s: %s Time: %s shmid: %s\n", argv[0], argv[1], argv[2], argv[3]); 


//	printf("Child Process\n"); 	

	//Declare shmptr 
	struct sharedMemory *shmptr; 
	
	//Set shmptr from shmid
	shmptr = (struct sharedMemory *) shmat(atoi(argv[3]), NULL, 0); 

/*
 *	Access 2D array datanumber
 *	Dynamically Allocate -> int *arr = malloc(Rows*Columns*sizeof(datatype));
 *	Access -> arr[i*M + j] rather than arr[i][j] 
 */
	//Test Shared Memory Access
	int i;  
	int arrLength = shmptr->leaves+1; 
	
	shmptr->leaves += 1; 
		
	printf("Array Length: %d\n", shmptr->leaves); 

//	for(i = 0; i < arrLength; ++i){
		
//		printf("[%d] ", shmptr->datanumber[i]); 
//	}

	shmptr->currProc--; 
	
	//Test
	printf("Child -> Concurrent Proc = %d\n", shmptr->currProc); 
	
	//Free pointer
	shmdt( shmptr ); 

	return EXIT_SUCCESS; 
}
