#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_CHILD 20

static volatile int mask = 0;
static volatile int terminated_children = 0;
static volatile int is_parent = 1;
static volatile int is_child = 1;
static volatile int with_signals = 0;
static volatile int array[NUM_CHILD] = {0};
static volatile int count = 0;

static void signal_handler(int sig) {
	switch (sig) {
		case SIGCHLD: {
			int status;
			pid_t pid;
			while ((pid = waitpid(-1, &status, 0)) > 0) {
				printf("parent[%d]: terminated child[%d].\n", getpid(), pid);
				terminated_children++;
			}
			break;
		}
		case SIGINT: {
			if (is_parent && with_signals) {
				mask = 1;
				printf("\nparent[%d]: recieved SIGINT.\n", getpid());
			}
			break;
		}
		case SIGTERM: {
			if (is_child && with_signals) {
				signal(SIGTERM, SIG_IGN);
				printf("child[%d]: recieved SIGTERM, terminating.\n", getpid());
			}
			break;	      
		}
		default:
			printf("No signal to catch.\n");
			break;
	}

}



int main(int argc, char *argv[]) {
	if (argc == 2 && strcmp(argv[1], "WITH_SIGNALS") == 0) {
		with_signals = 1;
		struct sigaction act;
		
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		act.sa_handler = signal_handler;

		if (sigaction(SIGCHLD, &act, NULL) == -1) {
			perror ("Error while handling SIGCHLD.\n");
			exit(1);
		}
		
		if (sigaction(SIGINT, &act, NULL) == -1) {
			perror ("Error while handling SIGINT.\n");
			exit(1);
		}

		if (sigaction(SIGTERM, &act, NULL) == -1) {
			perror ("Error while handling SIGTERM.\n");
			exit(1);
		}
	}
	pid_t pid;
	for (int i = 0; i < NUM_CHILD; i++) {
		pid = fork();

		if (pid<0) {
			perror("Error while creating process.\n");
			kill(0, SIGTERM);
			exit(0);
		}
		else if (pid == 0) {
			is_parent = 0;
			is_child = 1;
			if (with_signals) {
				for (int s=0; s<31; s++) {
					signal(s, SIG_IGN);
				}
				signal(SIGCHLD, SIG_DFL);
				signal(SIGTERM, SIG_DFL);
			}

			printf("child[%d]: parent process id is %d.\n", getpid(), getppid());
			sleep(10);
			printf("child[%d]: excution completed.\n", getpid());
			exit(0);
		}
		else {
			is_parent = 1;
			is_child = 0;
			if (with_signals) {
				if (mask !=0 ) {
					printf("parent[%d]: sending SIGTERM.\n", getpid());
					for (int term=0; term<count; term++) {
						kill(array[term], SIGTERM);
					}
					mask = 0;
				}
				else{
					array[count] = pid;
				}
				count++;
			}	
			printf("parent[%d]: created a child process.\n", getpid());
		}
		sleep(1);
	}

	for (;;) {
		pid_t end = wait(NULL);
		if (end == -1) {
			printf("parent[%d]: no more processes to be synchronized with the parent.\n", getpid());
			printf("parent[%d]: number of terminated children is %d.\n", getpid(), terminated_children);
			exit(0);
		}
	}
	
	
	return 0;
}
