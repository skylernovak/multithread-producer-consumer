#include <pthread.h>	/* for pthread functions - linux */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		// needed for sleep()
#include "buffer.h"

buffer_item buffer[BUFFER_SIZE];	// producer/consumer buffer
int in;								// position of next available in on buffer
int out;							// position of next available out on buffer
int count; 							// count on pg 258									!!! Do I need this? !!!
sem_t full;							// number in buffer, start at 0
sem_t empty;						// number available in buffer, start at 
sem_t mutex;						// binary semaphore, mutual exclusion access to buffer

/*
	insert item into buffer
	return 0 if successful
	return -1 if an error occured
*/
int insert_item(buffer_item item) {

	int pos = in;

	sem_wait(&empty);	// check if some buffer space is empty, then decrement empty space by 1
	sem_wait(&mutex);	// if mutex is 1 then makes it 0, then process enters critical section
	
	// Critical section start
	buffer[in] = item;
	in = (in + 1) % BUFFER_SIZE;
	count++;
	// Critical section end
	
	sem_post(&mutex);	// set mutex to 1
	sem_post(&full);	// increment full_sem by 1

	// output results
	printf("Insert_item inserted item %d at position %d\n", item, pos);

	// print buffer
	int i;
	for (i = 0; i < BUFFER_SIZE; i++) {
		if (buffer[i] != 0)
			printf("[%d]", buffer[i]);
		else
			printf("[EMPTY]");
	}
	printf(" in = %d, out = %d\n", in, out);

	return 0;

} // insert_item()

/*
	remove an object from buffer and place in item
	return 0 if successful
	return -1 if an error occured
*/
int remove_item(buffer_item *item) {

	int pos = out;

	sem_wait(&full);		// check if some buffer space contains item, then decrement it by 1
	sem_wait(&mutex);		// if mutex is 1, decrement to 0, then enter critical section
	
	// Critical section start
	item = &buffer[out];
	buffer[out] = 0;
	out = (out + 1) % BUFFER_SIZE;
	count--;
	// Critical section end
	
	sem_post(&mutex); 	// set mutex to 1
	sem_post(&empty); 	// increment empty by 1

	// output results
	printf("Remove_item removed item %d at position %d\n", *item, pos);

	// print buffer
	int i;
	for (i = 0; i < BUFFER_SIZE; i++) {
		if (buffer[i] != 0)
			printf("[%d]", buffer[i]);
		else
			printf("[EMPTY]");
	}
	printf(" in = %d, out = %d\n", in, out);

	return 0;

} // remove_item()

/*
	producer threads start here
*/
void *produce( void *ptr ) {

	buffer_item item;
	int id;
	id = *((int *) ptr);

	while(1){

    	/* generate random number */
		item = ((rand() % 50) + 1);

		/* sleep for random amount of time */
		int sleepTime = (rand() % 4) + 1;
		printf("Producer thread %d sleeping for %d seconds\n", id, sleepTime);
		sleep(sleepTime);

		// enter critical section, return error if failed
      	if (insert_item(item) < 0) {
        	printf("Producer error\n");
      	}

		// output successful display message
		printf("Producer thread %d inserted value %d\n", id, item);

	} // while(true)

} // produce()

/*
	Consumer threads start here
*/
void *consume( void *ptr ) {

	buffer_item item;
	int id;
	id = *((int *) ptr);
	
	while(1){

		/* sleep for random amount of time */
		int sleepTime = (rand() % 4) + 1;
		printf("Consumer thread %d sleeping for %d seconds\n", id, sleepTime);
		sleep(sleepTime);

		if (remove_item(&item))
			printf("Report error condition\n");
			
		// output successful display message
		printf("Consumer thread %d removed value %d\n", id, item);

	} // while(true)

} // consumer()

int main(int argc, char* argv[]){

	// declare start of execution
	printf("\n\n");
	printf("Main thread beginning\n");

	int i;							// for loop iteration
	pthread_attr_t attr; 			/* set of attributes for the thread */
	int runtime; 					// first main argument, how long main sleeps
	int numProducers; 				// second main argument, number of producers 
	int numConsumers; 				// third main argument, number of consumers

	/* 1. Get command line arguments */
	runtime = atoi(argv[1]);		// how long main runs until exit()
	numProducers = atoi(argv[2]);	// number of producer processes
	numConsumers = atoi(argv[3]);	// number of consumer processes

	/* 2. Initialize buffer & semaphores */
	in = out = count = 0;
	sem_init(&mutex, 0, 1); 			// initailizing mutex to 1, 2nd parameter 0 means mutex can be shared only between threads of same process
	sem_init(&full, 0, 0); 				// initailizing full to 0 because initially no items in buffer
	sem_init(&empty, 0, BUFFER_SIZE); 	// initailizing empty to BUFFER_SIZE since initially all the buffer are empty

	/* get the default attributes */
	pthread_attr_init(&attr);

	/* 3. Create producer threads. */
	pthread_t producers[numProducers]; 	/* the thread identifier */
	for (i = 0; i < numProducers; i++) { 
		/* Note that we are passing the thread id by reference */
		printf("Creating producer thread with id %lu\n", producers[i]);
		pthread_create(&producers[i],&attr,produce,(void *) &i); 
    }

	/* 4. Create consumer threads.  */
	pthread_t consumers[numConsumers]; 	/* the thread identifier */
	for (i = 0; i < numConsumers; i++) { 
		/* Note that we are passing the thread id by reference */
		printf("Creating consumer thread with id %lu\n", consumers[i]);
		pthread_create(&consumers[i],&attr,consume,(void *) &i); 
    }

	/* 5.  Sleep. */
	printf("Main thread sleeping for %d seconds\n", runtime);
	sleep(runtime);

	/* 6.  Main thread exits.  */
	printf("Main thread exiting\n");
	exit(0);

} // main()