#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h> //Edited. <wait.h> cannot found
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>


int main(int argc,char *argv[]){

	/* Implement the functions here */
	
	int status;

	pid_t pid;
	pid=fork();

	if (pid==-1) {
		perror("fork");
		exit(1);
	}
	/* fork a child process */
	else {
		if (pid==0) {
			char *arg[argc];
			for (int i=0;i<argc-1;i++) {
				arg[i]=argv[i+1];
			}
			arg[argc-1]=NULL;

			/* execute test program */ 
			execve(arg[0],arg,NULL);
			perror("execve");
			raise(SIGCHLD);
			exit(EXIT_FAILURE);
		}
		/* wait for child process terminates */
		else {
			waitpid(pid,&status,0);

			exit(1);
		}
		return 0;
	}
	return 0;
}
