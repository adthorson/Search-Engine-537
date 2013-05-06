
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "index.h"

#define TRUE 1
#define FALSE 0

char argc_error[83] = "Invalid amount of args!\nUsage: search-engine [indexer_thread_amount] [file-list]\n\n\0";
char unable_to_open[34] = "Invalid file or unable to open!\n\n\0";
char mutex_one_error[28] = "Semaphore mutex_one error!\n";
char full_error[30] = "Semaphore full_buffer error!\n";
char empty_error[31] = "Semaphore empty_buffer error!\n";
char index_thread_error[35] = "Unable to create indexer threads!\n";

char ** scan_buffer;

pthread_t * indexer_threads;

sem_t mutex_one;
sem_t full_buffer;
sem_t empty_buffer;

int * can_index;
int buffer_amount;
int indexer_amount;


void indexing(void * id){
	do{
		
	}while(!feof(file_list));
}



void addToBuffer(char * file_name){
	//printf("%s",buffer);
	scan_buffer[buffer_amount] = malloc((strlen(file_name)+1) * sizeof(char));
	scan_buffer[buffer_amount] = file_name;
	printf("%s",scan_buffer[buffer_amount]);
	can_index[buffer_amount] = TRUE;
	++buffer_amount;
}



int activateIndexerThreads(){
	int i;

	indexer_threads = malloc(indexer_amount * sizeof(pthread_t));	
	can_index = malloc(indexer_amount * sizeof(int));
	for(i = 0; i < indexer_amount; ++i){
		can_index[i] = FALSE;
		if(pthread_create(&indexer_threads[i], NULL, &indexing, i) != 0)
			return 1;	
	}
	
}



int main(int argc, char * argv[]) {

	// Let's try to guarantee proper usage
	if (argc != 3) {
		write(STDERR_FILENO, argc_error, strlen(argc_error));
		exit(1);
	}

	FILE * file_list;
	file_list = fopen(argv[2],"r");
	
	// If file is invalid or isn't loaded for some reason, there is no point of moving on
	if (file_list == NULL) {
		write(STDERR_FILENO, unable_to_open, strlen(unable_to_open));
		exit(1);
	}
	
	// How many threads will be indexing
	indexer_amount = atoi(argv[1]);
	scan_buffer = malloc(indexer_amount * sizeof(char *));

	char buffer[512];
	int line_number;
	char * word;
	char * save_ptr;

	//printf("BEFORE INIT\n");
	if(sem_init(&mutex_one, 0, 1) != 0){
		write(STDERR_FILENO, mutex_one_error, strlen(mutex_one_error));
		exit(1);
	}
	if(sem_init(&full_buffer, 0, 0) != 0){
		write(STDERR_FILENO, full_error, strlen(full_error));
		exit(1);
	}
	if(sem_init(&empty_buffer, 0, 1) != 0){
		write(STDERR_FILENO, empty_error, strlen(empty_error));
		exit(1);
	}

	// INDEXER
	if(activateIndexerThreads() != 0){
	 	write(STDERR_FILENO, index_thread_error, strlen(index_thread_error));
		exit(1);
	}

	// SCANNER
	// Read in file names from the list of files a.k.a. file_list
	while(!feof(file_list)){
		sem_wait(&empty_buffer);
		sem_wait(&mutex_one);
		printf("WAIT\n");
		buffer_amount = 0;
		while((fgets(buffer, 512, file_list) != NULL) && (buffer_amount < indexer_amount)){;
			addToBuffer(buffer);
		}
		sem_post(&mutex_one);
		sem_post(&full_buffer);
	}
        

	for(i = 0; i < indexer_amount; ++i)
                pthread_join(indexer_threads[i], NULL);

	fclose(file_list);

	return 0;
}
