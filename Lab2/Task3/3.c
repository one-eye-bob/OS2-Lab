#include <criu/criu.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h> 
#include <regex.h>

//The maximum number of matches allowed in a single string
int MAX_MATCHES =1;

//filename of the checkpoints
char filename[100];

//Reserved for the output file name
char* outputFN;

//Reserved for the checkpoint interval (in seconds)
int cpInterval;

//Reserved for the time of the last checkpoint
time_t last_time;

//Reserved for the already generated numbers
char* allNumbers = NULL;

int checkpoint(int signum) {
	//Reserved to check for functions' errors
	int ret = 0;

	char formatedTime[26];

	//Get the current time
	time_t t_ret = time(0);
	if(t_ret < 0)
		perror("Error: the function 'checkpoint' failed to set 't_ret'");

	ret = sprintf(filename, "%s.%d.dat", "checkpoint", t_ret);
	if (ret < 0){
		perror("Error: sprintf could not write\n");
		return;
	}

	//Get the output file or create a new one
	FILE* outputF = fopen(filename, "wb"); //open in wb mode so we overwrite every time
	//Check of the file could be opened
	if(outputF == NULL){
		perror("Error: fopen failed to open the \"checkpoint.dat\" file! \n");
		return -1;
	}

	//Print the state-string in the output file if allNumbers contains numbers
	if (allNumbers != NULL && allNumbers[0] != '\0') {
		ret = fprintf(outputF, "%s", allNumbers);
		if(ret < 0){
			perror("Error: fprintf could not write\n");
			return -1;
		}
	}

	//Close the output file
	ret = fclose(outputF);
	if(ret < 0){
		perror("Error: could not close the output file!\n");
		return -1;
	}

	printf("Wrote checkpoint %s\n", filename);
	return 0;
}

char* match_regex(regex_t *pexp, char *sz) {
	regmatch_t matches[MAX_MATCHES]; //A list of the matches in the string (a list of 1)
	//Compare the string to the expression
	//regexec() returns 0 on match, otherwise REG_NOMATCH
	//timestamp auf heap
	char* timestamp = malloc(sizeof(char) * 11);
	

	if (regexec(pexp, sz, MAX_MATCHES, matches, 0) == 0) {
		//printf("\"%s\" matches characters %d - %d\n", sz, matches[0].rm_so, matches[0].rm_eo);
		memcpy( timestamp, &sz[11], 10);
		timestamp[10] = '\0';
		//printf("match timestamp: %s\n",timestamp);
		return timestamp;
	} else {
		//printf("\"%s\" does not match\n", sz);
		return NULL;
	}
}

int getDataNames(char* dataNames ){
	DIR *d;
  	struct dirent *dir;
  	int i = 0;
  	d = opendir(".");
  	if (d)
  	{
    	while ((dir = readdir(d)) != NULL)
    		{
      			//printf("%s\n", dir->d_name);
				strcpy(dataNames+i*100,dir->d_name);
				i++;
    		}

    	closedir(d);
  	}else{
  		perror("Error while opening the dir");
  	}
  	return i;
}

int getTimestamp( int i, int timeStampArray[], char* dataNames, regex_t exp){
	char* matchRet;
	int l=0;
	for(int k=0;k<i;k++)
    {
    	char timestamp[15];
    	//printf(" %s \n",dataNames+k*100);
    	matchRet = match_regex(&exp, dataNames+k*100);
    	
		
    	
    	if(matchRet != 0){
    		timeStampArray[l] = atoi (matchRet);
    		//printf("match: %i \n", timeStampArray[l]);
    		l++;
    	}
    }
    return l;
}

int restore() {
	//Reserved to check for functions' errors
	int ret = 0;

	//Allocate allNumbers
	if (allNumbers==NULL) {
		//allocate memory for allNumbers
		allNumbers = malloc(100 * sizeof(char));
		allNumbers[0] = '\0';
	}

	int i;
  	char* dataNames = malloc(sizeof(char)*100*100);
  	i = getDataNames (dataNames);

	int rv;
	regex_t exp; // compiled expression
	// Compile expression.
	rv = regcomp(&exp, "checkpoint.*.dat", REG_EXTENDED);
	if (rv != 0) {
		printf("regcomp failed with %d\n", rv);
	}

	int timeStampArray[100];
	int l = getTimestamp(i, timeStampArray, dataNames, exp);
    // Free Expression
	regfree(&exp);

  	//search for latest Timestamp
  	int highestTimeStamp=0;
  
  	for(int m=0;m<l;m++)
    {
        if(timeStampArray[m]>highestTimeStamp)
        	highestTimeStamp=timeStampArray[m];

    }
	//printf("%i\n", highestTimeStamp);

	ret = sprintf(filename, "checkpoint.%i.dat", highestTimeStamp);
	if (ret < 0){
		perror("Error: sprintf could not write\n");
		return;
	}

	//Get the output file or create a new one
	FILE* inputF = fopen(filename, "r");

	//Check of the file could be opened
	if(inputF == NULL){
		perror("Error: fopen failed to open the checkpoint file or no checkpoint file found! \n");
		ret = working();
		return ret;
	}

	char inputText[100];

	ret = fread(inputText,sizeof(char),100,inputF);
	if(ret < 0){
		perror("Error: fread could not read\n");
		return -1;
	}
	//write \0 terminator
	//strncpy(inputText + ret,"\0",1);
	inputText[ret]='\0';

	//Close the input file
	ret = fclose(inputF);
	if(ret < 0){
		perror("Error: could not close the input file!\n");
		return -1;
	}

	strcpy(allNumbers,inputText);//*sizeof(char));

	printf("restored %s\n", allNumbers);

	ret = working();
	return ret;
}


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
	if(current_time < 0)
		perror("Error: the function 'time' failed to set 'current_time'");

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

void setCriuOptions(){
	//Reserved for the file descriptor
	int fd;

	//Open if "checkpoints" is a directory, or fail
	fd = open("checkpoints", O_DIRECTORY);

	//Check if open didn't fail
	if(fd == -1){
		perror("Error: open failed to open the 'checkpoints' directory.\n");
		//return -1;	
	}

	//Set up criu options
	int ret = criu_init_opts();
	if(ret == -1){
		perror("Error: criu_init_opts failed to inital the request options.\n");
	}

	//Set the images directory, where they will be stored
	criu_set_images_dir_fd(fd);

	//Specify the CRIU service socket
	criu_set_service_address("criu_service.socket");

	//Specify how to connnect to service socket
	criu_set_service_comm(CRIU_COMM_SK);
	
	//Make it to continue executing after dumping
	criu_set_leave_running(true);

	//Make it a shell job, since criu would be unable to dump otherwise
	criu_set_shell_job(true);
}

void makeACheckpoint(int signum){
	//Reserved to check for funtions's errors
	int ret;
	
	//Check if criu need to be initalized
	if(last_time == 0){
		//Preparation for CRIU for the first time:
	
		//Create the "checkpoints" directory if it doesn't exist, only owner has (full) access
		mkdir("checkpoints", 0700);

		//Set CRIU options
		setCriuOptions();
	}
	
	//Make a checkpoint
	ret = criu_dump();
	if(ret < 0){
		perror("Error: criu_dump failed!");
	} else{
		printf("Successfully took huge dump!\n");
	}
	
	//Set last_time to this time
	last_time = time(0);
	if(last_time < 0)
		perror("Error: the function 'time' failed to set 'last_time'");
}

int working(){
	//Defining the max number of character to be store in allNumbers
	int MAX_SIZE = 100;

	if (allNumbers==NULL) {
		//Not returning from a checkpoint, allocate memory for allNumbers
		allNumbers = malloc(MAX_SIZE * sizeof(char));
		allNumbers[0] = '\0';
	}

	//dump everytime we get a 10 signal
	struct sigaction dumpAction;
	dumpAction.sa_handler = checkpoint;
	sigaction(SIGUSR1, &dumpAction, NULL);
	printf("Worker: Starting to work!\n");
	
	//Defining the max number of characters as input (integer with '-')
	int MAXINPUT = 11;

	//Reserved for the first input, which should be integer
	char input[MAXINPUT];
					
	//Initial the range to get different seeds
	srand(getpid());

	//Loop forever and exit in some conditions
	while(1){
		//Tell the user to enter and type
		printf("$ ");

		//Read the input and check if no errors
		if(fgets(input, MAXINPUT, stdin) != NULL){
			//Check if a checkpoint should be taken
			/*if(isAllowedToCheckpoint()){
				//Make a checkpoint
				makeACheckpoint();
			}*/

			int n;

			//Convert the input to integer
			int parsed = sscanf(input, "%d", &n);
			
			//Check if conversion was successful 
			if(parsed >= 1){

				int i;

				//Check the input number
				if(n < 0)
					//Exit with returning the value n
					exit(n);
				else {

					//Create n random numbers
					for(i=0 ; i < n ; ++i){
						//Get a random number smaller than n
						int r = rand() % (n+1);

						//Concatenate the integer r with the return char
						char rStr[MAXINPUT];
						int ret = sprintf(rStr, "%d\n", r);
						if(ret < 0)
							perror("Error: sprintf failed");

						//TODO: check if failed or check MAX_SIZE
						if((strlen(allNumbers)+MAXINPUT+1) > MAX_SIZE)
							perror("Caution: The buffer of the generated numbers 'allNumbers' is full");
						//Add this new number(with return char) to all generated numbers						
						strncat(allNumbers, rStr, MAXINPUT);
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
			//exit(-1);
		}
	}
}

int monitoring(){
	int forkPID;
	bool useCheckpoint = false;
	int ret = 0;

	//Delete all checkpoint files
	int i;
  	char* dataNames = malloc(sizeof(char)*100*100);
  	i = getDataNames (dataNames);

	int rv;
	regex_t exp; // compiled expression
	// Compile expression.
	rv = regcomp(&exp, "checkpoint.*.dat", REG_EXTENDED);
	if (rv != 0) {
		printf("regcomp failed with %d\n", rv);
	}

	int timeStampArray[100];
	int l = getTimestamp(i, timeStampArray, dataNames, exp);
    // Free Expression
	regfree(&exp);

	for(int m=0;m<l;m++)
    {
    	ret = sprintf(filename, "checkpoint.%i.dat", timeStampArray[m]);
		if (ret < 0){
			perror("Error: sprintf could not write\n");
		return;
	}
        ret = unlink(filename);
			if (ret != 0){
				perror("Error: could not unlink!\n");
				continue;
			}

    }

	//Keep monitoring forever until the worker succeeds
	while(1){
		//Create a worker only if we didnt create a checkpoint
		if(!useCheckpoint){
			if((forkPID = fork()) < 0){
				perror("Error: fork couldn't create a child!");
				//Loop again
				continue;
			}
		}
		
		//Check if this process is the worker
		if(forkPID == 0)
			//Start the worker from scrach
			working();
		else {
			//Reserved for the status of the worker after it was terminated
			int status;

			int waitPID;
			do {
				printf("Monitor: Wait before making a checkpoint!\n");
				usleep(cpInterval*1000000);
				printf("Monitor: Send a siganl to make a checkpoint!\n");
				//time to send dump request
				kill(forkPID, SIGUSR1);
				//printf("Monitor: wait for the worker to respond!\n");
				//set an alarm, so the waiting process is regularly interrupted to send dump requests to the worker
				//alarm(cpInterval);	
				//Wait for the worker until it terminates
				waitPID = waitpid((pid_t)forkPID, &status, WNOHANG);
			//wait again if the process was interrupted by alarm
			} while (waitPID == 0 );
			
			//Wait for the worker until it terminates
			//waitPID = waitpid((pid_t)forkPID, &status, 0);

			if(waitPID < 0)
				perror("Error: waitpid couldn't let the monitor wait until the worker has finished");
			else if(WIFEXITED(status)) {
				if(WEXITSTATUS(status)) {
					/* //Commented for Task3
					//Monitor: This worker failed
					//Set CRIU options before restoring the last safe state
					setCriuOptions();

					//Restore the worker from last safe state
					if((forkPID = criu_restore_child()) < 0) {
						perror("Error: criu_restore_child failed to restore child!\n");
					}else{
						printf("Successfully restored child with pid: %d\n", forkPID);
						useCheckpoint = true;
					}*/
					//Restore the worker from last safe state
					forkPID = fork();
					if(forkPID < 0){
						perror("Error: fork couldn't create a child!");
						//Loop again
						continue;
					} 
					else if(forkPID==0) { //child
						restore();
					}
					else { //parent
						useCheckpoint = true;
					}
				} else {
					//Monitor: The worker succeeded!
					exit(0);
				}
					
			} else if (WIFSIGNALED(status) || WIFSTOPPED(status)) { //child killed by signal, sometimes get signal 10: BUS error (bad memory access).
	            perror("Child terminated unexpectedly!");
	            //Restore the worker from last safe state
				forkPID = fork();
				if(forkPID < 0){
					perror("Error: fork couldn't create a child!");
					//Loop again
					continue;
				} 
				else if(forkPID==0) { //child
					restore();
				}
				else { //parent
					useCheckpoint = true;
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