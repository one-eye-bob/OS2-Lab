#define _BSD_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

int MAX_FORKS; 
int fr=0; //failratio in percent

void logThis(char* text, int arg){
	int ret=0;

	// Get the log file or create a new one	
	FILE *logFile = fopen("log.txt", "a");
	
	// Check of the file could be opened
	if (logFile == NULL){
		perror("fopen failed to open log.txt file!\n");
		return;
	}

	// The string to be printed 
	char logText[100]; 
	ret = sprintf(logText, text, arg);
	if (ret < 0){
		perror("Error: sprintf could not write\n");
		return;
	}
	// Print the string in the log file
	ret = fprintf(logFile, "%s", logText);
	if (ret < 0){
		perror("Error: fprintf could not write\n");
		return;
	}
	// Close the log file
	ret = fclose(logFile);
	if (ret < 0){
		perror("Error: could not close file\n");
		return;
	}
}

void backup_terminated(int* fd){
	// Prepare the arguments in case the parent was terminated
	char arg1[10], arg2[10];
	sprintf (arg1 , "%s%d" , "-n " , MAX_FORKS);
	sprintf (arg2 , "%s%d" , "-f " , fr);
	
	// Try to write in the pipe
	char c[] = "test";
	int ret = write(fd[1], c, strlen(c)+1);

	// Check if it was not possible to write there
	if(ret == -1){
		// Check if the read end was closed -> parent was terminated
		if(errno == EPIPE){
			logThis("Child detects the termination of its parent and will take the role as a backup process!\n", 0);
			logThis("---\n", 0);
			logThis("The state of its parent was:\n", 0);
			logThis("Max Forks = %d\n", MAX_FORKS);
			logThis("Fail Ratio = %d\n", fr);
			logThis("---\n", 0);
			logThis("-----------------------------RESTARTING-----------------------------\n", 0);
			execl("run", "run", arg1, arg2, (char*)0);
      		perror("execl() failure!\n");
		} else
			perror("Could not write even through the parent was not terminated!");
	}
}

int server(int fr, int* fd){
	// Reserve memory for reading request files
	char buffer[255];
	struct dirent* entry;
	int ret=0;
	while(1){
		// Open a directory stream of requests
		DIR *requestdir	= opendir("./requests");
		
		// Loop on each file in this directory
		while((entry = readdir(requestdir)) != NULL){
			
			
			// Check if this file has only a regular type
			if (entry->d_type != DT_REG){
				continue;
			}
			
			// Open the file
			char filepath[100];
			strcpy(filepath, "requests/");
			strncat(filepath, entry->d_name, 22);
			FILE* requestFile = fopen(filepath, "r");
			if (requestFile == NULL){
				perror("Error: could not open file!\n");
				continue;
			}
			
			// Create artificial crash if specified by -f and filename contains "fail"
			const char failstr[10] = "fail";
			char* cret;
			cret = strstr(entry->d_name,failstr);
			if(fr > 0 && cret) {
				int r = rand() % 100;
				if (r >= fr) {
					printf("child process failed!\n");
					return -1;
				}
			}
			
			//only read in first line (max 255 chars), since this is only pseudo code
			char* request = fgets(buffer, 255, requestFile);
			char msg[100];
			sprintf(msg,"server [%d] req: %s\n", getpid(), entry->d_name);
			logThis(msg,0);
			ret = usleep(500000);
			if (ret < 0){
				perror("Error: usleep failed!\n");
				return ret;
			}
			//close and delete request files
			ret = fclose(requestFile);
			if (ret != 0){
				perror("Error: could not close file!\n");
				continue;
			}
			ret = unlink(filepath);
			if (ret != 0){
				perror("Error: could not unlink!\n");
				continue;
			}

			// Check if the parent process was terminated, otherweise continue
			backup_terminated(fd);
		}
		closedir(requestdir);
	}
}

int backup(int MAX_FORKS, int* fd) {
	int pid;
	int status;
	int sleep_status;
	int printParentID = 1; // will be used to check if parent ID was logged
	int num_forks = 0;

	// Create primary processes and wait in case they fail
	while(1){
		// Count number of attempts at child creation
		num_forks+=1;
		
		// Create a pipe to be used as a channel between parent and its child
		int ret = pipe(fd);
		if(ret == -1){
			perror("error when creating a pipe");
			return ret;
		}

		// Ignore the signal
		
		signal(SIGPIPE, SIG_IGN);

		printf("creating child...\n");
		pid = fork();

		// Handle fork error
		if(pid < 0) {
			perror("error when creating a child");
		}

		// Child process (primary)
		if(pid == 0) {
			// The child closes the write end of the pipe
			close(fd[0]);

			printf("I am the child process with pid %i\n", getpid());
			logThis("A new child process created with ID = %i\n", getpid());

			//return to start request processing
			return 0;
		}
		else if(pid < 0 && num_forks > MAX_FORKS){
			// Gracefully degrade and start request processing without backup
			printf("I am the parent process with pid %i\n",getpid());
			perror("Maximum number of retries to create child exceeded, backup process is now doing processing");
			return pid;
		}

		printf("I am the parent process with pid %i\n",getpid());
		
		// Check if the parent ID was not printed in the log file
		if(printParentID){
			logThis("The parent process has ID = %i\n", getpid());
			printParentID = 0;
		}
		
		// The parent closes the read end of the pipe
		close(fd[1]);

		// Wait for crash of primary
		printf("waiting %i\n", pid);
		pid = waitpid((pid_t)pid, &status, 0);
		if(pid < 0){
			perror("Error while trying to wait for child");
		}

		printf("done waiting%i\n", pid);
	}
	return 0;
}

int main(int argc, char** argv){
	printf("A nice welcome message\n");
	logThis("The program starts with a very nice welcome!\n", 0);
	int ret=0;

	int c; // Reserved for reading the program arguments
	int fd[2]; // Reserved for a pipe, the communication channel between parent and child
	
	while ((c = getopt(argc, argv, "n:f:")) != -1){
		switch(c){
			case 'n':
				MAX_FORKS=atoi(optarg);
				if(MAX_FORKS < 1 || MAX_FORKS >50){
					ret = fprintf(stderr, "Illegal MAX_FORKS argument (%i), set MAX_FORKS to 5\n", MAX_FORKS);
					if (ret < 0){
						perror("Error: fprintf could not write\n");
						return ret;
					}
					MAX_FORKS=5;
				}
				break;
			case 'f':
				fr = atoi(optarg);
				if (fr < 0 || fr > 100) {
					ret = fprintf(stderr, "Illegal failratio argument (%i), set failratio to 0\n", fr);
					if (ret < 0){
						perror("Error: fprintf could not write\n");
						return ret;
					}
					fr=0;
				}
				break;
			default:
				ret = fprintf(stderr, "Unknown or syntactically erroneous parameter\n");
				if (ret < 0){
					perror("Error: fprintf could not write\n");
					return ret;
				}
		}
	}

	// Logging program arguments
	logThis("---\n", 0);
	logThis("The program has the following arguments:\n", 0);
	logThis("Max Forks = %d\n", MAX_FORKS);
	logThis("Fail Ratio = %d\n", fr);
	logThis("---\n", 0);
	
	// Start creating processes
	backup(MAX_FORKS, fd);
	
	// Init rng after backup has been done to get different seeds
	srand(getpid());
	
	printf("start processing requests\n");
	ret = server(fr, fd);
	
	printf("done processing\n");
	exit(ret);
}
