#include <criu/criu.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

int main(){
	//Reserved for the file descriptor
	int fd;
	
	//Create the "dump" directory if it doesn't exist, only owner has (full) access
	mkdir("dump", 0700);
	
	//Open if "dump" is a directory, or fail
	fd = open("dump", O_DIRECTORY);
	
	//Check if open didn't fail
	if(fd == -1){
		perror("Error: open failed to open the 'dump' directory.\n");
		return -1;	
	}

	//Reserved to check the failures for the following functions
	int ret;
	
	//Preparation for CRIU:
	//Initial the request options
	ret = criu_init_opts();
	if(ret == -1){
		perror("Error: criu_init_opts failed to inital the request options.\n");
		return -1;
	}
	
	//Set the images directory, where they will be stored
	criu_set_images_dir_fd(fd);
	
	//Specify the CRIU service socket
	criu_set_service_address("criu_service.socket");
	
	//Specify how to connnect to service socket
	criu_set_service_comm(CRIU_COMM_SK);
	
	//To test whether the kernel support is up-to-dater
	ret = criu_check();
	if(ret < 0){
		perror("Error: criu_check failed.\n");
		return -1;
	}

	return 0;	
}
