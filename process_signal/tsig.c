#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

# define NUM_CHILD 5

// global variable using for all processes

// variable for checking occurrence of keyboard interrupt
static volatile int mask = 0;

// variable for counting number of terminated children
static volatile int terminated_children = 0;

// variable for checking process is parent or child
static volatile int is_parent = 1;
static volatile int is_child = 1;

// variable for checking program was complied with or without signals
static volatile int with_signals = 0;

// variable for storing process id of child process
static volatile pid_t array[NUM_CHILD] = {0};

// variable for counting index of just created processes
static volatile int count = 0;


// function handle signals
// SIGCHLD and SIGINT for parent process
// SIGTERM for child process
static void signal_handler(int sig) {
	switch (sig) {

		// handle SIGCHLD signal
		case SIGCHLD: {
			int status;
			pid_t pid;

			// waiting for SIGCHLD signal
			while ((pid = waitpid(-1, &status, 0)) > 0) {
				printf("parent[%d]: terminated child[%d].\n", getpid(), pid);
				// increase terminated children when parent recieved a SIGCHLD process
				terminated_children++;
			}
			break;
		}
		// handle SIGINT signal
		case SIGINT: {
			// only if the process is parent and in case "WITH_SIGNALS"
			if (is_parent && with_signals) {
				// change the mask for checking keyboard interrupt to 1, that means occurred
				mask = 1;
				// parent showing a message about recieving SIGINT signal
				printf("\nparent[%d]: recieved SIGINT.\n", getpid());
			}
			break;
		}
		// handle SIGTERM signal
		case SIGTERM: {
			// only if the process is child and in case "WITH_SIGNALS"
			if (is_child && with_signals) {
				// child showing a message about recieving SIGTERM signal
				printf("child[%d]: recieved SIGTERM, terminating.\n", getpid());
			}
			break;	      
		}
		// In case none of SIGCHLD, SIGINT or SIGTERM occurred
		default:
			printf("No signal to catch.\n");
			break;
	}

}


int main(int argc, char *argv[]) {
	
	// declare sigaction for calling process, allows process for handling the signal
	struct sigaction act;

	// reset all mask of signal, ensure that program can catch all the signal
	sigemptyset(&act.sa_mask);

	// set flags and handler for sigaction
	act.sa_flags = 0;
	act.sa_handler = signal_handler;

	// if complied with command line argument is WITH_SIGNALS
	if (argc == 2 && strcmp(argv[1], "WITH_SIGNALS") == 0) {
		with_signals = 1;
	}

	// create NUM_CHILD child process with the same parent
	pid_t pid;
	for (int i = 0; i < NUM_CHILD; i++) {
		pid = fork();
		
		// Can fork a new child process
		if (pid<0) {
			perror("Error while creating process.\n");
			// sending SIGTERM to all child process
			kill(0, SIGTERM);
			exit(1);
		}
		// If process is child process
		else if (pid == 0) {
			is_parent = 0;
			is_child = 1;

			// in case WITH_SIGNALS
			if (with_signals) {

				// ignore all SIGINT signal

				signal(SIGINT, SIG_IGN);

				// handler while recieved SIGTERM signal, if an error occurred, then exit
				if (sigaction(SIGTERM, &act, NULL) == -1) {
					perror ("Error while handling SIGTERM.\n");
					exit(1);
				}
			}
			
			// child process actions for both case with or without signals
			printf("child[%d]: parent process id is %d.\n", getpid(), getppid());
			sleep(10);
			printf("child[%d]: excution completed.\n", getpid());
			exit(0);
		}
		// If process is parent process
		else {
			is_parent = 1;
			is_child = 0;
			// in case WITH_SIGNALS
			if (with_signals) {
				
				// ignore all SIGNAL
				for (int s=0; s<31; s++) {
					signal(s, SIG_IGN);
				}

				// handler while recieved SIGCHLD signal, if an error occurred, then exit
				if (sigaction(SIGCHLD, &act, NULL) == -1) {
					perror ("Error while handling SIGCHLD.\n");
					exit(1);
				}

				// handler while recieved SIGINT signal, if an error occurred, then exit
				if (sigaction(SIGINT, &act, NULL) == -1) {
					perror ("Error while handling SIGINT.\n");
					exit(1);
				}

				// check if it has keyboard interrupt
				if (mask !=0 ) {

					// parent showing the message about sending SIGTERM to all just created children
					printf("parent[%d]: sending SIGTERM.\n", getpid());

					// sending SIGTERM sinal to all created child process
					// count is the current index of just created child process
					for (int term=0; term<count; term++) {
						kill(array[term], SIGTERM);
					}

					// assign mask = 0 for continueing with the normal creation process
					mask = 0;
				}
				// if no keyboard interrupt occurred
				else{
					// storing id of created child process to an array
					array[count] = pid;
				}

				// increasing the value of current index process
				count++;
			}	
			printf("parent[%d]: created child[%d].\n", getpid(), pid);
		}
		sleep(1);
	}
	
	// check to see if no more process to be synchronized with the parent
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
