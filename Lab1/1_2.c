#define _BSD_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<time.h>

int main() {
	int pid;
	int status;
	int sleep_status;
	
	printf("creating child...\n");
	pid = fork();
	sleep_status = usleep(250000);
	if (sleep_status < 0) {
		perror("error when sleeping");
        return sleep_status;
	}
	if(pid < 0) {
        perror("error when creating a child");
        return pid;
    }
    else if(pid == 0) {
        printf("i am the child process with pid %i\n",getpid());
        sleep_status = usleep(500000);
		if (sleep_status < 0) {
			perror("error when sleeping");
	        return sleep_status;
		}
			
		//init rng
		srand((unsigned) time(NULL));
		int r = rand() % 10;
		if (r >= 5) {
			printf("child process failed!\n");
			return -1;
		}
		printf("child process successfull\n");
    }
    else {
        printf("i am the parent process with pid %i\n",getpid());
        pid = waitpid(pid,&status,0);
        if (pid < 0) {
        	perror("error when waiting for child");
        	return pid;
        }
        if (WIFEXITED(status)) {
        	if (WEXITSTATUS(status) != 0) {
	        	printf("child process terminated unsuccesfully!\n");
	        	return -1;
        	}
        	printf("child process terminated!\n");
    	}
    }
	return 0;
}