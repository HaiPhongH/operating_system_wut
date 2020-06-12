#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define N 5	// Number of philosophers
#define LEFT (i + N - 1) % N	// The philosopher on the left	
#define RIGHT (i + 1) % N	// The philosopher on the right

#define THINKING 2
#define HUNGRY 1
#define EATING 0

sem_t m;	// for mutual exclusion
int state[N];	// philosophers' state
sem_t s[N];	// for synchronization

int phil[N] = {0, 1, 2, 3, 4};	// id of each philosophers


void test(int i) {
	// if philosopher i is hungry and both philosophers on the left and right is not eating
	// then philosopher i will take the forks on the left and right
	if( state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING )
	{	
		state[i] = EATING;	// philosopher i is eating
        	sleep(2);

		// philosopher i takes forks and use them for eating
        	printf("philosophers[%d]: takes fork %d and %d\n", i+1, LEFT + 1, i+1);
        	printf("philosopher[%d]: is eating\n", i+1);

		sem_post(&s[i]);	// wake a hungry philosopher up during put_away_forks
	}
}

// philosopher grabs forks
void grab_forks(int i) {
	sem_wait(&m);		// enter critical religion
	state[i] = HUNGRY;	// philosopher i is hungry
   	printf("philosopher[%d]: is hungry\n", i+1);
	test(i);		// eat if both philosophers on the left and right is not eating
	sem_post(&m);		// release critical religion
	sem_wait(&s[i]);	// if unable to eat, philosopher i will wait for a waking up
	sleep(1);
}

// philosopher put forks down
void put_away_forks(int i) {
	sem_wait(&m);		// enter critical religion
	state[i] = THINKING;	// philosopher i is thinking

	// philosopher i puts forks on the left and right down, and thinking after
	printf("philosohper[%d]: putting fork %d and %d down\n", i+1, LEFT + 1, i+1);
    	printf("philosoper[%d]: is thinking\n", i+1);

	test(LEFT);		// test eating conditions for philosopher on the left
	test(RIGHT);		// test eating cinditions for philosopher on the right
	sem_post(&m);		// release critical religion
}

// each philosopher is an endless cycle of thinking and eating
void *philosopher(void *num) {
	while(1) {
        	int *i = num;
        	sleep(1);
        	grab_forks(*i);		// philosopher i grabs forks on the left and right
        	sleep(1);		// eating
        	put_away_forks(*i);	// philosopher i puts forks on the left and right down
    	}
}

int main(int argc, char* argv[]){
    	int i;
    	pthread_t phi_threadID[N];	// philosopher thread id
	
    	sem_init(&m, 0, 1);		// init mutual exclusion

    	for (i = 0; i<N; i++) {
        	sem_init(&s[i], 0, 0);	// init philosopher semaphore for synchronization
    	}

    	for (i = 0; i< N; i++) {
		// create thread for each philosopher with id for each philosopher
        	pthread_create(&phi_threadID[i], NULL, philosopher, &phil[i]);
    	}
	
    	for (i = 0; i < N; i++) {
		// waiting for another thread to terminate
        	pthread_join(phi_threadID[i], NULL);
    	}
   	
	return 0;
}
