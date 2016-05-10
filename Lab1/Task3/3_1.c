#define _BSD_SOURCE
#include<stdio.h>
#include<dirent.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<time.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char** argv){
	printf("A nice welcome message\n");
	int c, MAX_FORKS;
	int fr=0; //failratio in percent
	int fd[2];
	
	//parse input
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
	backup(MAX_FORKS, fd);
	//init rng after backup has been done to get different seeds
	srand(getpid());
	printf("start processing\n");
	//start processing requests
	int ret = server(fr, fd);
	printf("done processing\n");
	exit(ret);

}
int server(int fr, int fd){
	//reserve memory for reading request files
	char buffer[255];
	struct dirent* entry;
	while(1){
		//call backup terminated before opening a new request
		int ret = backup_terminated(fd);

		//open directory stream
		DIR *requestdir	= opendir("./requests");
		while( (entry = readdir(requestdir)) != NULL){
			if (entry->d_type != DT_REG){
				continue;
			}
			char filepath[100];
			strcpy(filepath, "requests/");
			strncat(filepath, entry->d_name, 22);
			//only parse regular files
			FILE* requestFile = fopen(filepath, "r");
			
			//create artificial crash if specified by -f and filename contains "fail"
			const char failstr[10] = "fail";
			char* ret;
			ret = strstr(entry->d_name,failstr);
			if(fr > 0 && ret) {
				int r = rand() % 100;
				if (r >= fr) {
					printf("child process failed!\n");
					return -1;
				}
			}
			
			//only read in first line (max 255 chars), since this is only pseudo code
			char* request = fgets(buffer, 255, requestFile);
			printf("server [%d] req: %s\n", getpid(), entry->d_name);
			usleep(500000);
			fclose(requestFile);
			unlink(filepath);
		}
		closedir(requestdir);
	}
}
int backup(int MAX_FORKS, int fd) {
	int pid;
	int status;
	int sleep_status;
	int num_forks= 0;
	//create primary processes and wait in case they fail
	while(1){
		//count number of attempts at child creation
		num_forks+=1;
		printf("creating child...\n");
		pipe(fd);
		pid = fork();
		//handle fork error
		if(pid < 0) {
			perror("error when creating a child");
		}
		//child process(primary)
		if(pid == 0) {
			close(fd[1]);
			printf("i am the child process with pid %i\n",getpid());
			//return to start request processing
			return 0;
		}
		else if(pid < 0 && num_forks > MAX_FORKS){
			//gracefully degrade and start request processing without backup
			printf("i am the parent process with pid %i\n",getpid());
			perror("Maximum number of retries to create child exceeded, backup process is now doing processing");
			return pid;
		}

		close(fd[0]);
		//wait for crash of primary
		printf("waiting %i\n", pid);
		pid = waitpid((pid_t)pid,&status,0);
		printf("done waiting%i\n", pid);
		//handle waitpid error
		if(pid < 0){
			perror("Error while trying to wait for child");
		}
	}
	return 0;
}

int backup_terminated(int fd){
	int ret = write(fd, "hello", 5);
	printf("write feedback %i",ret);
}
