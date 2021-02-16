#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[]) {

//Testing Output using Fork and execl 

	printf("%s: %s Time: %d", argv[0], argv[2], argv[1]); 

	return EXIT_SUCCESS; 
}
