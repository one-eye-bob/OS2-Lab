#include <criu/criu.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>


//Reserved for the output file name
char* outputFN;

	
int writeOnOPFile(char* outputText){
	printf("Writing output to %s..", outputFN);

	//Reserved to check for functions' errors
	int ret = 0;

	//Get the output file or create a new one
	FILE* outputF = fopen(outputFN, "a");
		
	//Check of the file could be opened
	if(outputF == NULL){
		perror("Error: fopen failed to open a file with the following name: \n");
		perror(outputFN);
		return -1;
	}

	//Print the string in the output file
	ret = fprintf(outputF, "%s", outputText);
	if(ret < 0){
		perror("Error: fprintf could not write\n");
		return -1;
	}

	//Close the output file
	ret = fclose(outputF);
	if(ret < 0){
		perror("Error: could not close the output file!\n");
		return -1;
	}
		
	printf("Done.\n");

	return 0;
}

int working(){
	//Defining the max number of characters as input
	int MAX_SIZE = 11;

	//Reserved for the first input, which should be integer
	char input[MAX_SIZE];

	//To save the whole generated numbers as a string
	char* generatedNumbers = "";

	//Tell the user to enter to type
	printf("$");

	//Read the first input and check if no errors
	if(fgets(input, MAX_SIZE, stdin) != NULL){
		//TODO: check if input is number, like: isnumber(input)
		if(1){
			//Convert the input to integer
			int n = atoi(input);

			//Check in input number
			if(n < 0)
				//Exit with returning the value n
				return n;
			else if(n == 0){
				//Nothing to be done here
			} else {
				//Initial the range to get different seeds
				srand(getpid());

				//The expected summation for all natural number smaller than n
				int endSum = n*(n-1)/2;

				//Initial the current summation
				int curSum = 0;
				while(curSum < endSum){
					//Get a random number smaller than n
					int r = rand() % n;

					//Add only bigger than 0 
					if(r == 0)
						continue;
				
					//Concatenate the integer r with the return char
					char newNum[11];
					sprintf(newNum, "%d\n", r);

					//Check if this number already been added
					if(strstr(generatedNumbers, newNum) != NULL){
						//Ignore this old number
					} else {
						//Add this new number to the generated numbers string
						char tempStr[100];
						sprintf(tempStr, "%s%d\n", generatedNumbers, r);
						generatedNumbers = tempStr;

						//Add the new number in the current summation
						curSum += r;
						
						printf("%d\n", r);
					}
				}
			}
			printf("$");
			//Wait for any input
			char any[MAX_SIZE];
			fgets(any, MAX_SIZE, stdin);
		}
		//Write all generated numbers on the output file
		writeOnOPFile(generatedNumbers);
		
		//Return with the successful exit state
		return 0;
	} else {
		perror("Error: fgets failed!");
		return 1;
	}
}

int monitoring(){
	//Whether the work is done or continue monitoring 
	int keepWorking = 1;

	//keep monitoring for ever until the worker succeeds
	while(keepWorking){
		//Create a worker
		int forkPID = fork();
		if(forkPID < 0) {
			perror("Error: fork couldn't create a child!");
			//Loop again
			continue;
		}
		
		//Check if this process is the worker
		if(forkPID == 0){
			keepWorking = working();
			break;
		} else {
			//Reserved for the status of the worker after it was terminated
			int status;
			
			//Wait for the worker until it terminates
			int waitPID = waitpid((pid_t)forkPID, &status, 0);

			if(waitPID < 0){
				perror("Error: waitpid couldn't let the monitor wait until the worker has finished");
			} else if(WIFEXITED(status)){
				if(WEXITSTATUS(status)){
					//Monitor: This worker%i failed, I better hire a better one!
				} else {
					//Monitor: The worker succeeded!
					keepWorking = 0;
				}
			}
		}
	}

	return keepWorking;
}

int main(int argc, char** argv){
	//This process plays the monitoring role

	//Reserved for the output file
	FILE* outputF;

	//First get the name of the output file from the command line
	if(argc > 1){
		//Get the first argument to be the output file name
		outputFN = argv[1];

		/*TODO: these lines are not importnat, since writeOnOPFile will redo the same job!
		//Get the output file or create a new one
		outputF = fopen(outputFN, "a");
		
		//Check of the file could be opened
		if(outputF == NULL){
			perror("Error: fopen failed to open a file with the following name: \n");
			return -1;
		}*/
	} else {
		perror("Error: The program expected one argument to specify the output file name, nothing was given!.\n");
		return -1;	
	}

	//Start monitoring
	int ret = monitoring();
	
	exit(ret);	
}
