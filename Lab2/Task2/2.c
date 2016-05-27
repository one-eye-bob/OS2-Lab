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
		mkdir("checkpoints", 0700); //0700
	
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
		//continue executing after dumping
		criu_set_leave_running(true);
		//make it a shell job, since criu would be unable to dump otherwise
		criu_set_shell_job(true);
	}
	
	//Make a checkpoint
	ret = criu_dump();
	if(ret < 0){
		perror("Error: criu_dump failed!");
	} else{
		printf("Successfully took huge dump!\n");
	}
	//criu_set_shell_job(false);	
	//Set last_time to this time
	last_time = time(0);
}

int working(){
	//Defining the max number of character to be store in allNumbers
	int MAX_SIZE = 100;

	//Reserved for the whole generated numbers as a string
	char allNumbers[MAX_SIZE];

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
			int n;
			//Convert the input to integer
			int parsed = sscanf(input,"%d", &n);
			
			//Check if conversion was successful 
			if(parsed >= 1){
				//To save the following generated numbers as a string
				char* genNums = "#";

				//Convert the input to integer
				//int n = atoi(input);

				//Check in input number
				if(n < 0)
					//Exit with returning the value n
					exit(n);
				else {
					//Initial the range to get different seeds
					srand(getpid());

					int i;
					//create n random numbers
					for(i=0 ; i < n ; ++i){
						//Get a random number smaller than n
						int r = rand() % (n+1);

						//Concatenate the integer r with the return char
						//TODO: check if failed or check MAX_SIZE
						char rStr[MAX_INPUT];
						sprintf(rStr, "%d\n", r);
						strncat(allNumbers, rStr, MAX_INPUT);
						//sprintf(allNumbers, "#%d#", r);
						printf("%d\n", r);
					}
				}
				
			} else {

				//Write all generated numbers on the output file
				writeOnOPFile(allNumbers);
				//Return with the successful exit state
				exit(0);
			}
		} else {
			perror("Error: fgets failed!");
			exit(-1);
		}
	}
}

int monitoring(){
	int forkPID;
	bool useCheckpoint = false;
	//keep monitoring for ever until the worker succeeds
	while(1){
		//Create a worker only if we didnt create a checkpoint
		if(!useCheckpoint) {
		       forkPID = fork();
		}
		if(forkPID < 0) {
			perror("Error: fork couldn't create a child!");
			//Loop again
			continue;
		}
		
		//Check if this process is the worker
		if(forkPID == 0){
			//Start the worker from scrach
			working();
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
					criu_init_opts();
					//Open if "checkpoints" is a directory, or fail
					int fd = open("checkpoints", O_DIRECTORY);
					
					//Set the images directory, where they will be stored
					criu_set_images_dir_fd(fd);
					//Specify the CRIU service socket
					criu_set_service_address("criu_service.socket");
				
					//Specify how to connnect to service socket
					criu_set_service_comm(CRIU_COMM_SK);
					//continue executing after dumping
					criu_set_leave_running(true);
					//make it a shell job, since criu would be unable to dump otherwise
					criu_set_shell_job(true);
					//TODO: only works if checkpoint already exists
					if(( forkPID = criu_restore_child()) <0) {
						perror("failed to restore child! \n!");
					}else{
						printf("Successfully restored child with pid:%d!\n", pid);
						useCheckpoint=true;
					}
				} else {
					//Monitor: The worker succeeded!
					exit(0);
				}
			}
		}
	}

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
			if(sscanf(argv[2], "%d", &cpInterval) != 1){
							perror("Error: The program expected on the 2nd argument only a number!.\n");
				return -1;
			}
		}
	} else {
		perror("Error: The program expected on the 1st argument to specify the output file name, nothing was given!.\n");
		return -1;	
	}

	//Start monitoring
	monitoring();
	
}
