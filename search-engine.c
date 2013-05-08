
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

pthread_cond_t empty_cond, fill_cond;
pthread_mutex_t mutex_one;
//sem_t mutex_one;
//sem_t full_buffer;
//sem_t empty_buffer;
int finished_activations;

int buffer_amount, scan_buffer_size, scanning_done;
FILE * file_list;

void startScanning();
void startIndexing();
void createIndexThreads(pthread_t * threads, int num_threads);
void addToBuffer(char * file_name, int buffer_amount);
void removeFromBuffer(int pos);


void createIndexThreads(pthread_t * threads, int num_threads)
{
    int i;
    
    for (i=0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, startIndexing, i);
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
    
    printf("File to be indexed: %s\n", file_name);
    if ((file = fopen(file_name, "r")) == NULL) {
        printf("Wasn't able to open: %s\n", file_name);
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
	printf("FILE CLOSED!\n");
}


/*
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
 
 if (fgets(buffer, 512, file_list) != NULL) {
 if(buffer[strlen(buffer) - 1] == '\n')
 buffer[strlen(buffer) - 1] = '\0';
 //printf("\nGot here %d\n",buffer_amount);
 addToBuffer(buffer, buffer_amount);
 ++buffer_amount;
 }
 sem_post(&mutex_one);
 sem_post(&full_buffer);
 
 }
 scanning_done = TRUE;
 fclose(file_list);
 }
 */



void startScanning()
{
	char buffer[513];
    
	while(!feof(file_list)) {
		
        pthread_mutex_lock(&mutex_one);
		while(buffer_amount == scan_buffer_size)
			pthread_cond_wait(&empty_cond, &mutex_one);
        
		printf("AFTER WAIT\n");
        
		if (fgets(buffer, 512, file_list) != NULL) {
			if(buffer[strlen(buffer) - 1] == '\n')
				buffer[strlen(buffer) - 1] = '\0';
			printf("Add %d\n",buffer_amount);
            addToBuffer(buffer, buffer_amount);
            ++buffer_amount;
        }
		pthread_cond_signal(&fill_cond);
        pthread_mutex_unlock(&mutex_one);
	}
	pthread_mutex_lock(&mutex_one);
	scanning_done = TRUE;
    pthread_cond_broadcast(&fill_cond);
	pthread_mutex_unlock(&mutex_one);
	fclose(file_list);
}



/*
 void startIndexing()
 {
 while (!scanning_done || (buffer_amount != 0)) {
 int i;
 
 printf("Waiting to index\n");
 sem_wait(&full_buffer);
 sem_wait(&mutex_one);
 printf("Begin to index\n");
 
 // Implement whatever algo for multiple indexing threads
 // this one is for one thread, so it is linear
 
 removeFromBuffer(buffer_amount - 1);
 --buffer_amount;
 
 sem_post(&mutex_one);
 sem_post(&empty_buffer);
 }
 }
 */
void startIndexing(void * thread_number)
{
	int temp = (int) thread_number;
	while (TRUE) {
		printf("%d:Waiting to index\n",temp);
		pthread_mutex_lock(&mutex_one);
        printf("BUFFER AMOUNT = %d\n",buffer_amount);
		while(buffer_amount == 0){
            if(scanning_done){
                printf("BrOk3!\n");
                //pthread_cond_broadcast(&fill_cond);
                pthread_mutex_unlock(&mutex_one);
                return;
            }
            //printf("TED IS A SILLY LIL BOY\n");
			pthread_cond_wait(&fill_cond, &mutex_one);
        }
        
		printf("Begin to index\n");
        
        // Implement whatever algo for multiple indexing threads
        // this one is for one thread, so it is linear
		printf("\nRemove %d\n",buffer_amount);
		removeFromBuffer(buffer_amount - 1);
		--buffer_amount;
        
		pthread_cond_signal(&empty_cond);
		pthread_mutex_unlock(&mutex_one);
	}
}


int main(int argc, char * argv[])
{
    int i;
	buffer_amount = 0;
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
    
	scan_buffer = malloc(indexer_amount * 10 * sizeof(char *));
	int j;
	for(j = 0; j < indexer_amount * 10; ++j){
		scan_buffer[j] = malloc(513 * sizeof(char *));
	}
    
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
	/*if(sem_init(&mutex_one, 0, 1) != 0) {
     write(STDERR_FILENO, mutex_one_error, strlen(mutex_one_error));
     exit(1);
     }*/
	/*if(sem_init(&full_buffer, 0, 0) != 0) {
     write(STDERR_FILENO, full_error, strlen(full_error));
     exit(1);
     }
     if(sem_init(&empty_buffer, 0, indexer_amount * 10) != 0) {
     write(STDERR_FILENO, empty_error, strlen(empty_error));
     exit(1);
     }*/
	pthread_mutex_init(&mutex_one, NULL);
	pthread_cond_init(&empty_cond, NULL);
	pthread_cond_init(&fill_cond, NULL);
    
    finished_activations = 0;
    
    pthread_t index_threads[indexer_amount];
    createIndexThreads(index_threads, indexer_amount);
    
    startScanning();
    
    for (i=0; i < indexer_amount; i++) {
        printf("JOIN %d?\nINDEX Thread %u\n\n",i,index_threads[i]);
        if(pthread_join(index_threads[i], NULL))
            printf("HERE's YOUR PROBLEM!\n");
    }
    
    
	return 0;
}
