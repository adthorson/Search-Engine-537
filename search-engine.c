
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "index.h"

#define TRUE 1
#define FALSE 0


struct parameter {
    char test[5];
};

char argc_error[83] = "Invalid amount of args!\nUsage: search-engine [indexer_thread_amount] [file-list]\n\n\0";
char unable_to_open[34] = "Invalid file or unable to open!\n\n\0";
char mutex_one_error[28] = "Semaphore mutex_one error!\n";
char full_error[30] = "Semaphore full_buffer error!\n";
char empty_error[31] = "Semaphore empty_buffer error!\n";

char ** scan_buffer;

sem_t mutex_one;
sem_t full_buffer;
sem_t empty_buffer;
int finished_activations;

int buffer_amount, scan_buffer_size, scanning_done;
FILE * file_list;

void startScanning();
void startIndexing();
void createIndexThreads(pthread_t * threads, int num_threads);
void addToBuffer(char * file_name, int buffer_amount);
void removeFromBuffer(int pos);


/*
 * Activates a single (or multiple) thread by passing in the following parameters.
 * All threads will stay active until they all concurrently complete.
 *
 * @param 	int 	number_of_threads 	: the number of times we want to create this thread
 * 		  	void 	function_name		: the function to activate
 *			struct	parameter			: a struct of all parameter the specific function needs
 *
 * possible bugs: parameters may not have been written with correct pointers (& or *)
 * will fix when we test
 *
 void activate_Multi_Threader(int number_of_threads, void * function_name, struct parameter param) {
 pthread_t tidp;
 int i, thread0;
 for (i = 0; i < number_of_threads; i++) {
 //int  pthread_create(pthread_t  *  thread, pthread_attr_t * attr, void *(*start_routine)(void *), void * arg);
 thread0 = pthread_create(&tidp, NULL, function_name, (void *) &param);
 if (thread0 != 0) printf("\ncan't create thread\n");
 }
 if(finished_activations)
 pthread_join(tidp, NULL);
 }
 //NOTE NOTE NOTE
 //in order to pass in a method to a threader, YOU NEED TO name your method in this manner
 //add_to_buffer(char * filename)  needs to be
 //void* add_to_buffer(char * filename)
 */

void createIndexThreads(pthread_t * threads, int num_threads)
{
    int i;
    
    for (i=0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, startIndexing, NULL);
    }
}

void addToBuffer(char * file_name, int buffer_amount)
{
    strncpy(scan_buffer[buffer_amount], file_name, sizeof(file_name)-1);
    // USE STRCOPY WHEN INSERTING INTO SCAN_BUFFER
    strcpy(scan_buffer[buffer_amount], file_name);
	printf("%s", scan_buffer[buffer_amount]);
}

void removeFromBuffer(int pos)
{
    char * file_name;
    FILE * file;
    char buffer[512];
    int line_number;
    
    file_name = scan_buffer[pos];
    
    printf("File to be indexed: %s", file_name);
    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Wasn't able to open: %s", file_name);
        return;
    }
    while (!feof(file)) {
        line_number = 0;
        char * word;
        char * saveptr;
        fgets(buffer, 512, file);
        word = strtok_r(buffer, " \n\t-_!@#$%^&*()_+=,./<>?", &saveptr);
        while (word != NULL) {
            insert_into_index(word, file_name, line_number);
            word = strtok_r(NULL, " \n\t-_!@#$%^&*()_+=,./<>?",&saveptr);
        }
        line_number = line_number+1;
    }
    fclose(file);
}

void startScanning()
{
    int i;
    char buffer[513];
	char * copy;
	char ** save_ptr;
    
    while(!feof(file_list)) {
		sem_wait(&empty_buffer);
		sem_wait(&mutex_one);
        
		printf("WAIT\n");
		buffer_amount = 0;
        for (buffer_amount = 0; buffer_amount < scan_buffer_size; buffer_amount++) {
            if (fgets(buffer, 512, file_list) != NULL) {
                if(buffer[strlen(buffer) - 1] == '\n')
                    buffer[strlen(buffer) - 1] = '\0';
                printf("\nGot here %d\n",buffer_amount);
                addToBuffer(buffer, buffer_amount);
            }
        }
		sem_post(&mutex_one);
		sem_post(&full_buffer);
        
        printf("\nCHECK 1\n");
        for (i=0; i < 10; i++) {
            printf("scan_buffer[%d]: %s\n",i,scan_buffer[i]);
        }
	}
    scanning_done = TRUE;
	fclose(file_list);
}


void startIndexing()
{
    while (!scanning_done) {
        int i;
        
        printf("Waiting to index\n");
        sem_wait(&full_buffer);
        sem_wait(&mutex_one);
        printf("Begin to index\n");
        
        // Implement whatever algo for multiple indexing threads
        // this one is for one thread, so it is linear
        for (i=0; i < scan_buffer_size; i++) {
            removeFromBuffer(i);
        }
        
        sem_post(&mutex_one);
        sem_post(&empty_buffer);
    }
}

int main(int argc, char * argv[])
{
    int i, j;
    scanning_done = FALSE;
    
	// Let's try to guarantee proper usage
	if (argc != 3) {
		write(STDERR_FILENO, argc_error, strlen(argc_error));
		exit(1);
	}
    
	file_list = fopen(argv[2],"r");
	
	// If file is invalid or isn't loaded for some reason, there is no point of moving on
	if (file_list == NULL) {
		write(STDERR_FILENO, unable_to_open, strlen(unable_to_open));
		exit(1);
	}
	
	// How many threads will be indexing
	int indexer_amount = atoi(argv[1]);
    
    // Initialize hashtable
    int hash_init = init_index();
    if (hash_init != 0)
        printf("Index initialization error");
    
    // Malloc scan_buffer
    scan_buffer_size = indexer_amount * 10;
    scan_buffer = malloc(scan_buffer_size * sizeof(char *));
    for (i = 0; i < scan_buffer_size; ++i) {
        scan_buffer[i] = malloc(513 * sizeof(char *));
    }
    
	//printf("BEFORE INIT\n");
	if(sem_init(&mutex_one, 0, 1) != 0) {
		write(STDERR_FILENO, mutex_one_error, strlen(mutex_one_error));
		exit(1);
	}
	if(sem_init(&full_buffer, 0, 0) != 0) {
		write(STDERR_FILENO, full_error, strlen(full_error));
		exit(1);
	}
	if(sem_init(&empty_buffer, 0, 1) != 0) {
		write(STDERR_FILENO, empty_error, strlen(empty_error));
		exit(1);
	}
    finished_activations = 0;
    
    pthread_t index_threads[indexer_amount];
    createIndexThreads(index_threads, indexer_amount);
    
    startScanning();
    
    for (i=0; i < indexer_amount; i++) {
        pthread_join(index_threads[i], NULL);
    }
    
    //struct parameter param;
    //activate_Multi_Threader(indexer_amount, startIndexing, param);
    //activate_Multi_Threader(1, startScanning, param);
    //finished_activations = 1;
    //activate_Multi_Threader(1, SEARCH, param);
    
	return 0;
}
