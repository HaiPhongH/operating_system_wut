#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_CHILD 20
static volatile int mask = 0;
static int term_child[NUM_CHILD] = {0};
static volatile int array[NUM_CHILD] = {0};
static volatile int count = 0;

static void ignore_all_signal(int sig) {
}

void interrupt_handler(int sig) {
	//signal(SIGINT, SIG_DFL);
	mask = 1;
	printf("\nparent[%d]: recieved SIGINT.\n", getpid());
}
void terminate_handler(int sig) {
	signal(SIGTERM, SIG_IGN);
	printf("child[%d]: recieved SIGTERM, terminating.\n", getpid());
}

int check_term(int pid, int array[]) {
	for (int i=0; i<NUM_CHILD; i++) {
		if (pid == array[i]) {
			return 1;
		}
	}

	return 0;
}
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

void child_function_without_signal() {
	printf("child[%d]: created from parent[%d].\n", getpid(), getppid());
	sleep(10);
	printf("child[%d]: excution completed.\n", getpid());
	exit(0);
}
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

void parent_function_without_signal(int pid) {
	printf("parent[%d]: creating child[%d]...\n", getpid(), pid);
}
int main (int argc, char *argv[]) {
	
	pid_t pid;
	int terminated_children = 0;

	for (int i=0; i<NUM_CHILD; i++) {
		pid = fork();
		sleep(1);
		if (pid < 0) {
			printf("parent[%d]: Error while creating process.\n", getpid());
			kill(0, SIGTERM);
			exit(1);
		}
		else if (pid == 0) {
			if (argc == 2 && strcmp(argv[1], "WITH_SIGNALS") == 0) {
				child_function_with_signal();
			}
			else {
				child_function_without_signal();
			}
		}
		else {
			if (argc == 2 && strcmp(argv[1], "WITH_SIGNALS") == 0) {
				parent_function_with_signal(pid);
			}
			else {
				parent_function_without_signal(pid);
			}
		}
	}
	
	int status;
	for(;;) {
		pid_t end = wait(&status);
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
		terminated_children ++;
	}	
	return 0;
}
