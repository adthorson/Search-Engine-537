
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "index.h"


char argc_error[83] = "Invalid amount of args!\nUsage: search-engine [indexer_thread_amount] [file-list]\n\n\0";
char unable_to_open[34] = "Invalid file or unable to open!\n\n\0";
char mutex_one_error[28] = "Semaphore mutex_one error!\n";
char full_error[30] = "Semaphore full_buffer error!\n";
char empty_error[31] = "Semaphore empty_buffer error!\n";

char * scan_buffer[10];

sem_t mutex_one;
sem_t full_buffer;
sem_t empty_buffer;

int buffer_amount;



void addToBuffer(char * file_name){
	//printf("%s",buffer);
	scan_buffer[buffer_amount] = malloc((strlen(file_name)+1) * sizeof(char));
	scan_buffer[buffer_amount] = file_name;
	printf("%s",scan_buffer[buffer_amount]);
	++buffer_amount;
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
	int indexer_amount = atoi(argv[1]);

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


	// SCANNER
	// Read in file names from the list of files a.k.a. file_list
	while(!feof(file_list)){
		sem_wait(&empty_buffer);
		sem_wait(&mutex_one);
		printf("WAIT\n");
		buffer_amount = 0;
		while((fgets(buffer, 512, file_list) != NULL) && (buffer_amount < 10)){;
			addToBuffer(buffer);
		}
		sem_post(&mutex_one);
		sem_post(&full_buffer);
	}
        

	fclose(file_list);

	return 0;
}
