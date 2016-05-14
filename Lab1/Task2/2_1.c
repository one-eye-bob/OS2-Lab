#define _BSD_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

void logThis(char* text, int arg){
	// Get the log file or create a new one	
	FILE *logFile = fopen("log.txt", "a");
	
	// Check of the file could be opened
	if (logFile == NULL){
		perror("fopen failed to open log.txt file!\n");
		return;
	}

	// The string to be printed 
	char logText[100]; 
	sprintf(logText, text, arg);
	
	// Print the string in the log file
	fprintf(logFile, "%s", logText);
	
	// Close the log file
	fclose(logFile);
}

int server(int fr){
	//reserve memory for reading request files
	char buffer[255];
	struct dirent* entry;
	while(1){
		//open directory stream
		DIR *requestdir	= opendir("./requests");
		//read in and process files (=requests)
		while( (entry = readdir(requestdir)) != NULL){
			//only parse regular files
			if (entry->d_type != DT_REG){
				continue;
			}
			//open request file
			char filepath[100];
			strcpy(filepath, "requests/");
			strncat(filepath, entry->d_name, 22);
			FILE* requestFile = fopen(filepath, "r");
			
			//create artificial crash if specified by -f and filename contains "fail"
			const char failstr[10] = "fail";
			char* ret;
			ret = strstr(entry->d_name,failstr);
			if(fr > 0 && ret) {
				int r = rand() % 100;
				if (r >= fr) {
					logThis("child process failed!\n",0);
					return -1;
				}
			}
			
			//only read in first line (max 255 chars), since this is only pseudo code
			char* request = fgets(buffer, 255, requestFile);
			char msg[100];
			sprintf(msg,"server [%d] req: %s\n", getpid(), entry->d_name);
			logThis(msg,0);
			usleep(500000);
			//close and delete request files
			fclose(requestFile);
			unlink(filepath);
		}
		closedir(requestdir);
	}
}
int backup(int MAX_FORKS) {
	int pid;
	int status;
	int sleep_status;
	int num_forks= 0;
	//create primary processes and wait in case they fail
	while(1){
		//count number of attempts at child creation
		num_forks+=1;
		logThis("creating child...\n",0);
		pid = fork();
		//handle fork error
		if(pid < 0) {
			perror("error when creating a child");
		}
		//child process(primary)
		if(pid == 0) {
			logThis("i am the child process with pid %i\n",getpid());
			//return to start request processing
			return 0;
		}
		else if(pid < 0 && num_forks > MAX_FORKS){
			//gracefully degrade and start request processing without backup
			logThis("i am the parent process with pid %i\n",getpid());
			perror("Maximum number of retries to create child exceeded, backup process is now doing processing");
			return pid;
		}
		//wait for crash of primary
		logThis("waiting %i\n", pid);
		pid = waitpid((pid_t)pid,&status,0);
		logThis("done waiting%i\n", pid);
		//handle waitpid error
		if(pid < 0){
			perror("Error while trying to wait for child");
		}
	}
	return 0;
}


int main(int argc, char** argv){
	printf("A nice welcome message\n");
	int c, MAX_FORKS;
	int fr=0; //failratio in percent
	
	//parse input: n sets maximum number of server spawning attempts
	//f: fail ratio
	while ((c = getopt(argc, argv, "n:f:")) != -1){
		switch(c){
			case 'n':
				MAX_FORKS=atoi(optarg);
				if(MAX_FORKS < 1 || MAX_FORKS >50){
					fprintf(stderr, "Illegal MAX_FORKS argument (%i), set MAX_FORKS to 5\n", MAX_FORKS);
					MAX_FORKS=5;
				}
				break;
			case 'f':
				fr = atoi(optarg);
				if (fr < 0 || fr > 100) {
					fprintf(stderr, "Illegal failratio argument (%i), set failratio to 0\n", fr);
					fr=0;
				}
				break;
			default:
				fprintf(stderr, "Unknown or syntactically erroneous parameter\n");
		}
	}
	
	//start creating processes
	backup(MAX_FORKS);
	//init rng after backup has been done to get different seeds
	srand(getpid());
	logThis("start processing\n",0);
	//start processing requests
	int ret = server(fr);
	logThis("done processing\n",0);
	exit(ret);

}
