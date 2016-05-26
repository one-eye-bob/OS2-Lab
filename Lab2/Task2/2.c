#include <criu/criu.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


//Reserved for the output file name
char* outputFN;

//Reserved for the checkpoint interval (in seconds)
int cpInterval;

//Reserved for the time of the last checkpoint
time_t last_time;

	
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

int is_number(char* string){
	//TODO

	//It is a number
	return 1;
}

int isAllowedToCheckpoint() {
	//Reserved for the current time
	time_t current_time;

	//Reserved to calculate the differnce in seconds
	int difference;

	//Check if the last time was NOT set
	if(last_time == 0){
		//Take the first checkpoint
		return 1;
	}

	//Get the current time
	current_time = time(0);

	//Calculate the difference between these times (in seconds)
	difference = current_time - last_time;
	
	//Is the last checkpoint time old
	if(difference >= cpInterval){
		//Free to take a new checkpoint
		return 1;
	}

	//It is too soon
	return 0;
}

int makeACheckpoint(){
	//TODO: I need to test this function
	//Reserved to check for funtions's errors
	int ret;
	
	//Check if criu need to be initalized
	if(last_time == 0){
		//Reserved for the file descriptor
		int fd;
	
		//Create the "checkpoints" directory if it doesn't exist, only owner has (full) access
		mkdir("checkpoints", 7777); //0700
	
		//Open if "checkpoints" is a directory, or fail
		fd = open("checkpoints", O_DIRECTORY);
	
		//Check if open didn't fail
		if(fd == -1){
			perror("Error: open failed to open the 'checkpoints' directory.\n");
			return -1;	
		}

		//Set up criu options
		ret = criu_init_opts();
		if(ret == -1){
			perror("Error: criu_init_opts failed to inital the request options.\n");
		}
	
		//Set the images directory, where they will be stored
		criu_set_images_dir_fd(fd);
	
		//Specify the CRIU service socket
		criu_set_service_address("criu_service.socket");
	
		//Specify how to connnect to service socket
		criu_set_service_comm(CRIU_COMM_SK);
	}
	
	//Make a checkpoint
	ret = criu_dump();
	if(ret < 0){
		//TODO: I always get 'ret' = -52
		perror("Error: criu_dump failed!");
	}
	
	//Set last_time to this time
	last_time = time(0);
}

int working(){
	//Reserved for the whole generated numbers as a string
	char* allGeneratedNumbers = "";

	//Defining the max number of character to be store in allGeneratedNumbers
	int MAX_SIZE = 100;

	//Defining the max number of characters as input (integer with '-')
	int MAX_INPUT = 11;

	//Reserved for the first input, which should be integer
	char input[MAX_INPUT];

	//Loop forever until and exit in some conditions
	while(1){
		//Tell the user to enter to type
		printf("$");

		//Read the first input and check if no errors
		if(fgets(input, MAX_INPUT, stdin) != NULL){
			//Check if a checkpoint should be taken
			if(isAllowedToCheckpoint()){
				//Make a checkpoint
				makeACheckpoint();
			}

			//Check if input is number
			if(is_number(input)){
				//To save the following generated numbers as a string
				char* genNums = "#";

				//Convert the input to integer
				int n = atoi(input);

				//Check in input number
				if(n < 0)
					//Exit with returning the value n
					return n;
				else {
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
						char newNum[MAX_INPUT+1];
						sprintf(newNum, "#%d#", r);

						//Check if this number already been added
						if(strstr(genNums, newNum) != NULL){
							//Ignore this old number
						} else {
							//Add this new number to the current generated numbers string
							char temp1[MAX_SIZE], temp2[MAX_SIZE];
							sprintf(temp1, "%s%d#", genNums, r);
							genNums = temp1;
							
							//As well the whole generated numbers
							sprintf(temp2, "%s%d\n", allGeneratedNumbers, r);
							allGeneratedNumbers = temp2;

							//Add the new number in the current summation
							curSum += r;
						
							printf("%d\n", r);
						}
					}
				}
			} else {
				//Write all generated numbers on the output file
				writeOnOPFile(allGeneratedNumbers);
		
				//Return with the successful exit state
				return 0;
			}
		} else {
			perror("Error: fgets failed!");
			return 1;
		}
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
			//TODO: Start from the last checkpoint (if any)
			
			//Start the worker from scrach
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
		
		//Get the interval for checkpoints (if it was given)
		if(argc > 2){
			//Check if the parameter is number
			if(is_number(argv[2])){
				//Convert it to integer
				cpInterval = atoi(argv[2]);
			} else {
				perror("Error: The program expected on the 2nd argument only a number!.\n");
				return -1;
			}
		}
	} else {
		perror("Error: The program expected on the 1st argument to specify the output file name, nothing was given!.\n");
		return -1;	
	}

	//Start monitoring
	int ret = monitoring();
	
	exit(ret);	
}