#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


void handle_signal(int signal);

int main(int argc, char *argv[]){
	int status;

	pid_t pid;
	printf("Process start to fork\n");
	pid=fork();

	if (pid==-1) {
		perror("fork");
		exit(1);
	}
	/* fork a child process */
	else {
		if (pid==0) {
			printf("I'm the child process, my pid is = %d \n", getpid());
			
			char *arg[argc];
			for (int i=0;i<argc-1;i++) {
				arg[i]=argv[i+1];
			}
			arg[argc-1]=NULL;

			/* execute test program */ 
			printf("Child process start to execute the program \n");
			execve(arg[0],arg,NULL);
			perror("execve");
			raise(SIGCHLD);
			exit(EXIT_FAILURE);
		}
		/* wait for child process terminates */
		else {
			printf("I'm the parent process, my pid = %d \n", getpid());
			waitpid(pid,&status,WUNTRACED);
			printf("Parent process receiving the SIGCHILD signal \n");

			/* check child process'  termination status */
			//Normal Termination
			if (WIFEXITED(status)) {
				printf("Normal termination with EXIT STATUS = %d\n", WEXITSTATUS(status));
			}
			else if (WIFSIGNALED(status)) {
				printf("CHILD EXECUTION FAILED: %d\n", WTERMSIG(status));
				if (WTERMSIG(status) == 1) {
					printf("child process get SIGHUP signal\n");
					printf("child process is terminated by hangup signal\n");
				}
				else if (WTERMSIG(status) == 2) {
					printf("child process get SIGINT signal\n");
					printf("child process is terminated by interrupt signal\n");
				}
				else if (WTERMSIG(status) == 3) {
					printf("child process get SIGQUIT signal\n");
					printf("child process is terminated by quit signal\n");
				}
				else if (WTERMSIG(status) == 4) {
					printf("child process get SIGILL signal\n");
					printf("child process is terminated by illegal instruction signal\n");
				}
				else if (WTERMSIG(status) == 5) {
					printf("child process get SIGTRAP signal\n");
					printf("child process is terminated by trap signal\n");
				}
				else if (WTERMSIG(status) == 6) {
					printf("child process get SIGABRT signal\n");
					printf("child process is terminated by abort signal\n");
				}
				else if (WTERMSIG(status) == 7) {
					printf("child process get SIGBUS signal\n");
					printf("child process is terminated by bus signal\n");
				}
				else if (WTERMSIG(status) == 8) {
					printf("child process get SIGFPE signal\n");
					printf("child process is terminated by floating point exception signal\n");
				}
				else if (WTERMSIG(status) == 9) {
					printf("child process get SIGKILL signal\n");
					printf("child process is terminated by kill signal\n");
				}
				else if (WTERMSIG(status) == 11) {
					printf("child process get SIGSEGV signal\n");
					printf("child process is terminated by segmentation violation signal\n");
				}
				else if (WTERMSIG(status) == 13) {
					printf("child process get SIGPIPE signal\n");
					printf("child process is terminated by pipe signal\n");
				}
				else if (WTERMSIG(status) == 14) {
					printf("child process get SIGAlRM signal\n");
					printf("child process is terminated by alarm signal\n");
				}
				else if (WTERMSIG(status) == 15) {
					printf("child process get SIGTERM signal\n");
					printf("child process is terminated by termination signal\n");
				}
				printf("CHILD EXECUTION FAILED!!\n");
			}
			else if (WIFSTOPPED(status)) {
				printf("child process get SIGSTOP signal\n");
				printf("child process stopped\n");
				printf("CHILD PROCESS STOPPED\n");
			}
			else {
				printf("CHILD PROCESS CONTINUED\n");
			}
			exit(0);
		}
		return 0;
	}

}

//Another possible solution to distinguish the signals received
void handle_signal(int signal) {
	const char *signal_name;
	sigset_t pending;

	//Find out which single we're handling
	switch (signal) {
	case SIGTERM:
	    signal_name = "SIGTERM";
		break;
	}
}