#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_CHILD 20

// mask variable to check interrupt
static volatile int mask = 0;

// array store id of terminated proccess after recieved interrupt signal
static int term_child[NUM_CHILD] = {0};

// array store
static volatile int array[NUM_CHILD] = {0};

// variable for counting number of terminated processes after
// recieved interrupt signal
static volatile int count = 0;


// function that handle to ignore all signal
static void ignore_all_signal(int sig) {
}


// function handle interrupt signal
void interrupt_handler(int sig) {
	//signal(SIGINT, SIG_DFL);
	mask = 1;
	printf("\nparent[%d]: recieved SIGINT.\n", getpid());
}

// function handle termination signal
void terminate_handler(int sig) {
	signal(SIGTERM, SIG_IGN);
	printf("child[%d]: recieved SIGTERM, terminating.\n", getpid());
}


// function check a process id is in array or not
int check_term(int pid, int array[]) {
	for (int i=0; i<NUM_CHILD; i++) {
		if (pid == array[i]) {
			return 1;
		}
	}

	return 0;
}

// child function for signal case compilation
void child_function_with_signal() {
	/*
	for (int i=0; i<31; i++) {
		signal(i, ignore_all_signal);
	}
	*/
	//signal(SIGCHLD, SIG_DFL);
	signal(SIGINT, SIG_IGN);
	printf("child[%d]: from parent[%d].\n", getpid(), getppid());
	sleep(10);
	if (check_term(getpid(), term_child) == 1) {
		signal(SIGTERM, terminate_handler);
	}
	else {
		printf("child[%d]: excution completed.\n", getpid());
	}
	kill(getppid(), SIGCHLD);
	exit(0);
}


// child function for non signal case compilation
void child_function_without_signal() {
	printf("child[%d]: created from parent[%d].\n", getpid(), getppid());
	sleep(10);
	printf("child[%d]: excution completed.\n", getpid());
	exit(0);
}

// parent function for signal case compilation
void parent_function_with_signal(int pid) {
	signal(SIGINT, interrupt_handler);
	if (mask != 0) {
		printf("parent[%d]: sending SIGTERM to child ...\n", getpid());
		for (int i=0; i<count; i++) {
			term_child[i] = array[i];
			kill(array[i], SIGTERM);
		}
		count++;
		mask = 0;
	}
	else {
		printf("parent[%d]: sending creation signal to child[%d]...\n", getpid(), pid);
		array[count] = pid;
		count++;
	}
}

// parent function for non-signal case compilation
void parent_function_without_signal(int pid) {
	printf("parent[%d]: creating child[%d]...\n", getpid(), pid);
}


int main (int argc, char *argv[]) {
	
	pid_t pid;
	int terminated_children = 0;
	
	// creating NUM_CHILD number of processes
	for (int i=0; i<NUM_CHILD; i++) {
		pid = fork();
		sleep(1);

		// case when can not create a new process
		if (pid < 0) {
			printf("parent[%d]: Error while creating process.\n", getpid());
			kill(0, SIGTERM);
			exit(1);
		}

		// case when process is child process
		else if (pid == 0) {

			// check command line parameter has "WITH_SIGNALS" or not
			// if it has, do the case with signal
			if (argc == 2 && strcmp(argv[1], "WITH_SIGNALS") == 0) {
				child_function_with_signal();
			}
			// else do the case with non signal
			else {
				child_function_without_signal();
			}
		}

		// case when process is parent process
		else {
			// check command line parameter has "WITH_SIGNALS" or not
			// If it has, do the case with sinal
			if (argc == 2 && strcmp(argv[1], "WITH_SIGNALS") == 0) {
				parent_function_with_signal(pid);
			}
			// else, do the case with non-signal
			else {
				parent_function_without_signal(pid);
			}
		}
	}
	
	int status;
	for(;;) {

		// parent waiting from system
		pid_t end = wait(&status);

		// if it has no child process
		if (end == -1) {
			printf("parent[%d]: no more processes to be synchronized with the parent.\n", getpid());
			printf("parent[%d]: number of terminated children is %d.\n", getpid(), terminated_children);
			printf("parent[%d]: a child has just terminated with exit code %d.\n", getpid(), status);
			exit(EXIT_FAILURE);
		}
		else if (end == pid) {
			if (WIFEXITED(status)) {
				printf("child[%d]: terminated normally.\n", pid);
			}
			else if (WIFSIGNALED(status)) {
				printf("child[%d]: terminated because of an uncaught signal.\n", pid);
			}
			else if (WIFSTOPPED(status)) {
				printf("child[%d]: stopped.\n", pid);
			}
		}
		//  count terminated processes
		terminated_children ++;
	}	
	return 0;
}
