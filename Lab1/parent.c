#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


int main(){
	printf("start\n");

	// Create a new child.
	int ret = fork();
	usleep(250000); // sleep for 250ms.

	// Check if the fork function has a problem.
	if(ret == -1){
		perror("fork could not create a child!");
		exit(EXIT_FAILURE);
	}
	
	// Get the process ID of the current process.
	int pid = getpid();
	printf("This process has this ID = %d \n", pid);
	
	
	if(ret == 0){
		// This process is a child:
		printf("This process is a child! \n");
		usleep(500000); // sleep for 500ms
	} else {
		// This process is the parent:
		printf("The parent has a child with ID = %d \n", ret);
		
		
		int childStatus, // The child status.
			waitpidRet; // The return value of waitpid.
		
		do {
			// Suspend execution of the parent process until its child has changed state.
			waitpidRet = waitpid(ret, &childStatus, 0|WUNTRACED); // possible options: |WNOHANG|WUNTRACED|WCONTINUED);
		
			if(waitpidRet == -1){
				perror("waitpid raises an error! ");
				
				// Chech for an appropriate error message and print it.
				if(errno == ECHILD)
					printf("The process or process group specified by pid does not exist or is not a child of the calling process.\n");
				else if(errno == EFAULT)
					printf("stat_loc is not a writable address.\n");
				else if(errno == EINTR)
					printf("The function was interrupted by a signal. The value of the location pointed to by stat_loc is undefined.\n");
				else if(errno == EINVAL)
					printf("The options argument is not valid.\n");
				else if(errno == ENOSYS)
					printf("pid specifies a process group (0 or less than -1), which is not currently supported.\n");
				
				exit(EXIT_FAILURE);
			}
		
			printf("The child status = %d \n", childStatus);
		
			// Check the child state and print an appropiate message.
			if(WIFEXITED(childStatus))
				printf("The child terminated normally! \n");
			else
				if(WIFSIGNALED(childStatus))
					printf("The child was killed by a signal! \n");
				else
					if(WIFSTOPPED(childStatus)) // WARNING: it works only if WUNTRACED was given in waitpid!
						printf("The child was stopped by delivery of a signal! \n");
					else
						if(WIFCONTINUED(childStatus))
							printf("The child was resumed by delivery of SIGCONT! \n");
						else
							printf("Unexpected status (0x%x)\n", childStatus);

		
		} while(!WIFEXITED(childStatus)&!WIFSIGNALED(childStatus)); // Loop if the process not terminated or forced to be.
	}

	printf("\n\n");
	return 0;
}
