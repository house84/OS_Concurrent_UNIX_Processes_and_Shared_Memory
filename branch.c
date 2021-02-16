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


int main(int argc, char * argv[]) {

//Testing Output using Fork and execl 

	printf("Child -> %s: %s Time: %d\n", argv[0], argv[2], argv[1]); 

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
	int i, j; 
	int rows = shmptr->depth + 1; 
	int col = shmptr->leaves; 
	for(i = 0; i < rows; ++i){
		for(j = 0; j < col; ++j){
			printf("%d\n", shmptr->datanumber[i*col + j]);
		}
	} 

	
	//Free pointer
	shmdt( shmptr ); 

	return EXIT_SUCCESS; 
}
