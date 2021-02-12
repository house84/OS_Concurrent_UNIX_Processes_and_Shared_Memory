#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

void help(char * program); 

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
		
			case 'i': printf("%s option i\n", optarg); 
				  children = atoi(optarg); 
				  //Test Setting
				  printf("Children: %d\n", children); 
				  break;
	
			case 't': printf("%s option t\n", optarg); 
				  time = atoi(optarg); 
				  //Test Setting
				  printf("Time: %d\n", time); 
				  break; 

			default: break;
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
	
	//Parse through file contents
	while((ch = fgetc(filepointer)) != EOF ){
		
		printf("%c", ch); 
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
		else if( flag_1 == 2 && ch != '\n' && ch != '\0' && ch != ' ' ){

			printf("\nERROR: Line: %d Index: %d Character: %c ->  Only one integer allowed per line\n", lines, characters, ch); 
			
			exit (EXIT_FAILURE); 
		}


		//Check New Line
		if( ch == '\n' ){
			
			lines++;
			printf("New Line -> Flag: %d\n", flag_1); 
			characters = 0;
			flag_1 = 0; 
		} 		
	}


	
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
	//Call on Child processes with datafile contents
	

	//      system("./branch");
	//
	//      Wait for Child Processes to finish
	//      while(wait(NULL) > 0);
 

	//Close datafile
	fclose(filepointer);


	return EXIT_SUCCESS; 
}


//Usage page -h Option 
void help(char * program){

	printf("\n======================================================== %s Usage Page ======================================================== \n", program);
	printf("\n%s -h\n", program);
	printf("%s [-h] [-s x] [-t time] datafile\n", program); 
	printf("\n%s -i x\t\t-> argument required for this option: x indicates the number of children allowed to exist (default 20)\n", program);  
	printf("%s -t time\t-> argument required for this option: time in seconds after the process will terminate even if not finished.\n", program);
	printf("%s datafile\t-> datafile required, this file must only contain integers, and only one integer per page. No blank lines.\n\n", program);
	printf("====================================================================================================================================== \n\n"); 
}
