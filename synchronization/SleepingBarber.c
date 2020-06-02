#include <unistd.h>          
#include <stdio.h>           
#include <stdlib.h>          
#include <pthread.h>         
#include <semaphore.h>       

#define MAX_CHAIRS 5            // No. of chairs in waiting room
#define CUT_TIME 3              // Hair Cutting Time in second

#define MAX_CUST 10             // Maximum no. of customers for simulation
#define NUM_BARB_M 3            // Maximum no. of barbers for man
#define NUM_BARB_W 1            // Maximum no. of barbers for woman
#define NUM_BARB_B 2            // Maximum no. of barbers for both

sem_t customers;                // Semaphore use for counting number of customer in shop          
sem_t barbersM;                 // Semaphore use for wake up a barber Man
sem_t barbersW;                 // Semaphore use for wake up a barber Woman
sem_t barbersB;                 // Semaphore use for wake up a barber Both

sem_t available_BM;             // Semaphore use for counting number of available barbers Man
sem_t available_BW;             // Semaphore use for counting number of available barbers Woman
sem_t available_BB;             // Semaphore use for counting number of available barbers Both

sem_t mutex;                    //Semaphore for providing mutially exclusive access

int numberOfFreeSeats = MAX_CHAIRS;     // Counter for vacant seats in waiting room
int seatPocket[MAX_CHAIRS];             // To exchange id between customer and barber
int sitHereNext = 0;                    // Index for next legitimate seat
int serveMeNext = 0;                    // Index to choose a candidate for cutting hair
static int count = 0;                   // Counter of No. of customers
int customer_gender[MAX_CHAIRS];        // Generate randomly gender. 0 is male, 1 is female

void barberForManThread(void *tmp);     // Thread Function of Barber for Man
void barberForWomanThread(void *tmp);   // Thread Function of Barber for Woman
void barberForBothThread(void *tmp);    // Thread Function of Barber for Both
void customerThread(void *tmp);         //Thread Function of Customer

int main(int argc, char *argv[])
{   
    // Array contains threads for each types of barber
    pthread_t barbers_for_man[NUM_BARB_M];
    pthread_t barbers_for_woman[NUM_BARB_W];
    pthread_t barbers_for_both[NUM_BARB_B];

    // Array contains threads for customer
    pthread_t customer[MAX_CUST];

    int i, status=0;

    // Generate randomly gender of a customer
    for (i=0; i<MAX_CUST; i++) {
        customer_gender[i] = rand() % 2;
    }

    /*Semaphore initialization*/
    sem_init(&customers, 0, 0);
    sem_init(&barbersM, 0, 0);
    sem_init(&barbersW, 0, 0);
    sem_init(&barbersB, 0, 0);

    sem_init(&available_BM, 0, NUM_BARB_M);
    sem_init(&available_BW, 0, NUM_BARB_W);
    sem_init(&available_BB, 0, NUM_BARB_B);

    sem_init(&mutex,0,1);

    /*Barber thread initialization*/
    printf("!!-----------Barber Shop Opens-----------!!\n");
    for(i=0;i<NUM_BARB_M;i++)                       // Creation of "N1" Barber Threads
    {   
       status=pthread_create(&barbers_for_man[i],NULL,(void *)barberForManThread,(void*)&i);
       sleep(1);
       if(status!=0)
          perror("No Barber for Man Present... Sorry!!\n");
    }

    for(i=0;i<NUM_BARB_W;i++)                       //Creation of "N2" Barber Threads
    {   
       status=pthread_create(&barbers_for_woman[i],NULL,(void *)barberForWomanThread,(void*)&i);
       sleep(1);
       if(status!=0)
          perror("No Barber for Man Present... Sorry!!\n");
    }

    for(i=0;i<NUM_BARB_B;i++)                       //Creation of "N3" Barber Threads
    {   
       status=pthread_create(&barbers_for_both[i],NULL,(void *)barberForBothThread,(void*)&i);
       sleep(1);
       if(status!=0)
          perror("No Barber for Man Present... Sorry!!\n");
    }

    /*Customer thread initialization*/
    for(i=0;i<MAX_CUST;i++)                         //Creation of Customer Threads
    {   
       status=pthread_create(&customer[i],NULL,(void *)customerThread,(void*)&i);
       sleep(rand() % 2);
       if(status!=0)
           perror("No Customers Yet!!!\n");
    }

    //Waiting till all customers are dealt with
    for(i=0;i<MAX_CUST;i++)                         
        pthread_join(customer[i],NULL);
    printf("!!-----------Barber Shop Closes-----------!!\n");
    exit(EXIT_SUCCESS);  
}

/*-----------Customer Process-----------*/
void customerThread(void *tmp)  
{   
    int mySeat, B;
    int freeBarbersM, freeBarbersW, freeBarbersB;
    sem_wait(&mutex);       //Lock mutex to protect seat changes
    count++;                //Arrival of customer
    printf("Customer[%d]: Entered Shop. ",count);

    if (customer_gender[count] == 0) {
        printf("Customer is a man. ");
    }
    else {
        printf("Customer is a woman. ");
    }
    if(numberOfFreeSeats > 0) 
    {
        --numberOfFreeSeats;                        //Sit on chairs on waiting room
        printf("Customer[%d]: Sits In Waiting Room.\n",count);

        sitHereNext = (++sitHereNext) % MAX_CHAIRS;  //Choose a vacant chair to sit
        mySeat = sitHereNext;
        seatPocket[mySeat] = count;

        sem_post(&mutex);                               //Release the seat change mutex

        if (customer_gender[count] == 0) {              // If Customer is a Man
            sem_getvalue(&available_BM, &freeBarbersM); // Getting value of available Barbers Man
            if (freeBarbersM > 0) {                     // If there is at least 1 available
                sem_post(&barbersM);                    //  Wake up one Barber Man
            }
            else {                                      // Else there is no one available
                sem_post(&barbersB);                    // Wake up one Barber Both
            }
        }
        else {                                          // If Customer is a Woman
            sem_getvalue(&available_BW, &freeBarbersW); // Getting value of available Barbers Woman
            if (freeBarbersW > 0) {                     // If there is at least 1 available
                sem_post(&barbersW);                    // Wake up one Barber woman
            }
            else {                                      // Else there is no one available
                sem_post(&barbersB);                    // Wake up one Barber Both
            }
        }
        sem_wait(&customers);                           // Join queue of sleeping customers
        sem_wait(&mutex);                               // Lock mutex to protect seat changes

        B = seatPocket[mySeat];                         // Barber replaces customer PID with his own PID
        numberOfFreeSeats++;                            // Stand Up and Go to Barber Room
        sem_post(&mutex);                               // Release the seat change mutex
        
        /*-------Customer is having hair cut by barber 'B'---------*/
    } 
    else 
    {
       sem_post(&mutex);  //Release the mutex and customer leaves without haircut
       printf("Customer[%d]: Finds No Seat & Leaves.\n",count);
    }
    pthread_exit(0);
}

/*-----------Barber for Man Process-----------*/

void barberForManThread(void *tmp)        
{   
    int index = *(int *)(tmp);      
    int myNext, C;
    printf("Barber(M)[%d]: Joins Shop. ",index);
    
    while(1)            /*Infinite loop*/   
    {   
        printf("Barber(M)[%d]: Gone To Sleep.\n",index);
        sem_wait(&barbersM);                            // Join queue of sleeping barbers
        sem_wait(&mutex);                               // Lock mutex to protect seat changes

        serveMeNext = (++serveMeNext) % MAX_CHAIRS;     // Select next customer
        myNext = serveMeNext;
        C = seatPocket[myNext];                         // Get selected customer's ID
        seatPocket[myNext] = (int) (long) pthread_self();     //Leave own ID for customer
        sem_post(&mutex);
        sem_post(&customers);                           // Call selected customer
        sem_wait(&available_BM);                        // Decrease number of available by 1
        
        /*Barber is cutting hair of customer 'C'*/
        printf("Barber(M)[%d]: Wakes Up & Is Cutting Hair Of Customer[%d].\n",index,C);
        sleep(CUT_TIME);
        printf("Barber(M)[%d]: Finishes. ",index);
        sem_post(&available_BM);                        // Increase number of available by 1
    }
}

/*-----------Barber for Woman Process-----------*/

void barberForWomanThread(void *tmp)        
{   
    int index = *(int *)(tmp);      
    int myNext, C;
    printf("Barber(W)[%d]: Joins Shop. ",index);
    
    while(1)            /*Infinite loop*/   
    {   
        printf("Barber(W)[%d]: Gone To Sleep.\n",index);
        sem_wait(&barbersW);                            // Join queue of sleeping barbers
        sem_wait(&mutex);                               // Lock mutex to protect seat changes

        serveMeNext = (++serveMeNext) % MAX_CHAIRS;     // Select next customer
        myNext = serveMeNext;
        C = seatPocket[myNext];                         // Get selected customer's ID
        seatPocket[myNext] = (int) (long) pthread_self();     //Leave own PID for customer
        sem_post(&mutex);
        sem_post(&customers);                           // Call selected customer
        sem_wait(&available_BW);                        // Decrease number of available by 1

        /*Barber is cutting hair of customer 'C'*/
        printf("Barber(W)[%d]: Wakes Up & Is Cutting Hair Of Customer[%d].\n",index,C);
        sleep(CUT_TIME);
        printf("Barber(W)[%d]: Finishes. ",index);
        sem_post(&available_BW);                        // Increase number of available by 1
    }
}

/*-----------Barber for Both Process-----------*/
void barberForBothThread(void *tmp)        
{   
    int index = *(int *)(tmp);      
    int myNext, C;
    printf("Barber(B)[%d]: Joins Shop. ",index);
    
    while(1)            /*Infinite loop*/   
    {   
        printf("Barber(B)[%d]: Gone To Sleep.\n",index);
        sem_wait(&barbersB);                            // Join queue of sleeping barbers
        sem_wait(&mutex);                               // Lock mutex to protect seat changes

        serveMeNext = (++serveMeNext) % MAX_CHAIRS;     // Select next customer
        myNext = serveMeNext;
        C = seatPocket[myNext];                         // Get selected customer's ID
        seatPocket[myNext] = (int) (long) pthread_self();     // Leave own ID for customer
        sem_post(&mutex);
        sem_post(&customers);                           // Call selected customer
        sem_wait(&available_BB);                        // Decrease number of available by 1
        /*Barber is cutting hair of customer 'C'*/
        printf("Barber(B)[%d]: Wakes Up & Is Cutting Hair Of Customer[%d].\n",index,C);
        sleep(CUT_TIME);
        printf("Barber(B)[%d]: Finishes. ",index);
        sem_post(&available_BB);                        // Increase number of available by 1
    }
}
