#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {

	printf("Master\n"); 
	
	system("./branch"); 

	//Wait for Child Processes to finish
	while(wait(NULL) > 0); 

	return EXIT_SUCCESS;
}
