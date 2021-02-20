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


	//Declare shmptr 
	struct sharedMemory *shmptr; 
	
	//Set shmptr from shmid
	shmptr = (struct sharedMemory *) shmat(atoi(argv[3]), NULL, 0); 
	
	//argv[1] xx
	int xx = atoi(argv[1]); 

	//argv[2] yy
	int yy = atoi(argv[2]); 
/*
 *	Access 2D array datanumber
 *	Dynamically Allocate -> int *arr = malloc(Rows*Columns*sizeof(datatype));
 *	Access -> arr[i*M + j] rather than arr[i][j] 
 */
	//Test Shared Memory Access
	//shmptr->dataArr[xx] = yy; 
	fprintf(stderr, "Array: %d\n", shmptr->dataArr[xx]); 
	
//	int i;   
//	for(i = 0; i < shmptr->leaves; ++i){
		
//		printf("Arr [%d] ", shmptr->dataArr[i] ); 
//	}

	shmptr->currProc--; 
	
	//Test
	printf("Child -> Concurrent Proc = %d\n", shmptr->currProc); 
	
	//Free pointer
//	shmdt( shmptr ); 

	return EXIT_SUCCESS; 
}
