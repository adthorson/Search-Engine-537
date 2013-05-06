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

char scan_buffer[10][512];

sem_t mutex_one;
sem_t full_buffer;
sem_t empty_buffer;

int buffer_amount, i;
FILE * file_list;


void addToBuffer(char * file_name, int buffer_amount)
{
    //if (file_name[strlen(file_name) - 1] == '\n') {
      //  file_name[strlen(file_name) - 1] == '\0';
    //}
    
    //char * dest;
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
        int line_number = 0;
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
    char buffer[513];
	char * copy;
	char ** save_ptr;
    
    while(!feof(file_list)) {
		sem_wait(&empty_buffer);
		sem_wait(&mutex_one);
        
		printf("WAIT\n");
		buffer_amount = 0;
        for (buffer_amount = 0; buffer_amount < 10; buffer_amount++) {
            if (fgets(buffer, 513, file_list) != NULL) {
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
        
        startIndexing();
	}
    
	fclose(file_list);
}


void startIndexing()
{
    sem_wait(&full_buffer);
    sem_wait(&mutex_one);
    
    // Implement whatever algo for multiple indexing threads
    // this one is for one thread, so it is linear
    for (i=0; i < 10; i++) {
        removeFromBuffer(i);
    }
    
    sem_post(&mutex_one);
    sem_post(&empty_buffer);
}


int main(int argc, char * argv[])
{
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
    
    
	printf("BEFORE INIT\n");
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
    
    startScanning();
    
	return 0;
}
