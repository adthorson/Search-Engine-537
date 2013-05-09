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
char hash_error[30] = "Index initialization error\n";

char ** scan_buffer;

pthread_cond_t empty_cond;
pthread_cond_t fill_cond;
pthread_cond_t indexed_cond;
pthread_cond_t indexed_noticed_cond;
pthread_cond_t searched_cond;

pthread_mutex_t mutex_one;
pthread_mutex_t mutex_advanced;
pthread_mutex_t mutex_test;

int finished_activations;

int buffer_amount;
int scan_buffer_size;
int scanning_done;
int adv_flag;
int indexing_done = FALSE;
int indexing_done_noticed = FALSE;
int quit_search = FALSE;
FILE * file_list;

void startScanning();
void startIndexing(void * thread_number);
void createIndexThreads(pthread_t * threads, int num_threads);
void addToBuffer(char * file_name);
void removeFromBufferAndIndex(int thread_number);
void startSearch();


void createIndexThreads(pthread_t * threads, int num_threads)
{
    int i;
    
    for (i=0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, startIndexing, i);
    }
}


void addToBuffer(char * file_name)
{
    strncpy(scan_buffer[buffer_amount], file_name, sizeof(file_name)-1);
    // USE STRCOPY WHEN INSERTING INTO SCAN_BUFFER
    strcpy(scan_buffer[buffer_amount], file_name);
    //printf("%s\n", scan_buffer[buffer_amount]);
    ++buffer_amount;
}


void removeFromBufferAndIndex(int thread_number){
    char * file_name;
    FILE * file;
    char buffer[512];
    int line_number = 1;
    
    // Waiting to remove from buffer
    //printf("STOP REMOVE!\n");
    pthread_mutex_lock(&mutex_one);
    //printf("IN REMOVE LOCK\n");
    while(buffer_amount == 0){
        if(scanning_done){
            pthread_mutex_unlock(&mutex_one);
            return;
        }
        pthread_cond_wait(&fill_cond, &mutex_one);
        //printf("STUCK IN REMOVE\n");
    }
    
    file_name = scan_buffer[buffer_amount - 1];
    --buffer_amount;
    //printf("--BUFFER_AMOUNT -> in removeFromBufferAndIndex\n");
    pthread_cond_signal(&empty_cond);
    pthread_mutex_unlock(&mutex_one);
    // Has removed from buffer and signal to others who are waiting
    
    
    //printf("Thread: %d  File to be indexed: %s\n", thread_number, file_name);
    if ((file = fopen(file_name, "r")) == NULL) {
        //printf("Wasn't able to open: %s\n", file_name);
        return;
    }
    while (!feof(file)) {
        char * word;
        char * saveptr;
        fgets(buffer, 512, file);
        word = strtok_r(buffer, " \n\t-_!@#$%^&*()_+=,./<>?", &saveptr);
        while (word != NULL) {
            //printf("Word inserted: %s\n",word);
            
            //TESTING PURPOSES ONLY (using old index.c)
            pthread_mutex_lock(&mutex_test);
            insert_into_index(word, file_name, line_number);
            
            //TESTING PURPOSES ONLY (using old index.c)
            pthread_mutex_unlock(&mutex_test);
            
            word = strtok_r(NULL, " \n\t-_!@#$%^&*()_+=,./<>?",&saveptr);
        }
        line_number = line_number+1;
    }
    fclose(file);
}


void startScanning()
{
	char buffer[513];
    int i = 0;
    
	while(!feof(file_list)) {
		
        pthread_mutex_lock(&mutex_one);
		while(buffer_amount > 0)
			pthread_cond_wait(&empty_cond, &mutex_one);
        
		for(i = 0; i < scan_buffer_size; ++i){
            if (fgets(buffer, 512, file_list) != NULL) {
                if(buffer[strlen(buffer) - 1] == '\n')
                    buffer[strlen(buffer) - 1] = '\0';
                //printf("Add %d\n",buffer_amount);
                addToBuffer(buffer);
                continue;
                // else not actually needed, but used as a safety net
            }else{
                break;
            }
        }
        
		pthread_cond_broadcast(&fill_cond);
        pthread_mutex_unlock(&mutex_one);
	}
    
	pthread_mutex_lock(&mutex_one);
	scanning_done = TRUE;
    pthread_cond_broadcast(&fill_cond);
	pthread_mutex_unlock(&mutex_one);
    
	fclose(file_list);
}



void startIndexing(void * thread_number){
	int temp = (int) thread_number;
	while (TRUE) {
        
		removeFromBufferAndIndex(thread_number);
        
        //while(buffer_amount == 0){
        pthread_mutex_lock(&mutex_one);
        if(scanning_done){
            pthread_mutex_unlock(&mutex_one);
            return;
        }
        pthread_mutex_unlock(&mutex_one);
        //pthread_cond_signal(&empty_cond);
        //}
	}
}

/*
 void advSearch(char * input)
 {
 index_search_results_t * all_results;
 index_search_elem_t result;
 int num_results;
 char * file_name;
 file_name = malloc(512);
 char * file_name_result;
 file_name_result = malloc(512);
 char * word;
 word = malloc(512);
 int i, found=0, line_number;
 
 word = strtok(input, " ");
 strcpy(file_name, word);
 word = strtok(NULL, " ");
 
 printf("FILE: %s WORD: %s\n", file_name, word);
 
 all_results = find_in_index(word);
 if (all_results == NULL) {
 printf("Word not found.\n");
 }
 else {
 num_results = all_results->num_results;
 for (i=0; i < num_results; i++) {
 result = all_results->results[i];
 line_number = result.line_number;
 file_name_result = result.file_name;
 int cmp = strncmp(file_name_result, file_name, 512);
 if (cmp != 0) {
 continue;
 }
 printf("FOUND: %s %d\n", file_name_result, line_number);
 found = 1;
 }
 }
 if (!found)
 printf("Word not found.\n");
 }
 */

void advSearch(char * input)
{
    index_search_results_t * all_results;
    index_search_elem_t result;
    int num_results;
    char * file_name;
    file_name = malloc(512);
    char * file_name_result;
    file_name_result = malloc(512);
    char * word;
    word = malloc(512);
    int i, found=0, line_number;
    int adv_flag = FALSE;
    
    word = strtok(input, " ");
    strcpy(file_name, word);
    word = strtok(NULL, " ");
    
    printf("FILE: %s WORD: %s\n", file_name, word);
    
    while(1){
        adv_flag = FALSE;
        all_results = find_in_index(word);
        if (all_results != NULL) {
            num_results = all_results->num_results;
            for (i=0; i < num_results; i++) {
                result = all_results->results[i];
                line_number = result.line_number;
                file_name_result = result.file_name;
                int cmp = strncmp(file_name_result, file_name, 512);
                if (cmp != 0) {
                    continue;
                }
                printf("FOUND: %s %d\n", file_name_result, line_number);
                found = 1;
            }
            break;
        }
        
        pthread_mutex_lock(&mutex_advanced);
        while(!indexing_done){
            printf("WAITING FOR INDEXING\n");
            pthread_cond_wait(&indexed_cond, &mutex_advanced);
            adv_flag = TRUE;
            printf("SHOULD BE INDEXED");
        }
        printf("ADVANCED: INDEXING IS DONE");
        pthread_mutex_unlock(&mutex_advanced);
        if(!adv_flag)
            break;
    }
    if (!found){
        printf("Word not found.\n");
    }
}


void basicSearch(char * input)
{
    index_search_results_t * all_results;
    index_search_elem_t result;
    int num_results;
    char * file_name;
    int i, line_number;
    
    printf("%s\n", input);
    
    all_results = find_in_index(input);
    if (all_results == NULL) {
        printf("Word not found.\n");
    }
    else {
        num_results = all_results->num_results;
        for (i=0; i < num_results; i++) {
            result = all_results->results[i];
            line_number = result.line_number;
            file_name = result.file_name;
            printf("FOUND: %s %d\n", file_name, line_number);
        }
    }
}


void startSearch()
{
    char input[130];
    index_search_results_t * all_results;
    index_search_elem_t result;
    int num_results;
    char * file_name;
    int i, c, adv, line_number;
    char tst[512];
    
    while((c = getchar()) != EOF) {
        printf("SEARCH\n");
        input[0] = c;
        adv = FALSE;
        printf("Search: ");
        fgets(tst, sizeof(tst), stdin);
        for (i=1; i < strlen(tst); i++) {
            input[i] = tst[i-1];
        }
        input[i] = '\0';
        if(input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';
        for (i=0; i < strlen(input); i++) {
            if (input[i] == ' ') {
                adv = TRUE;
                break;
            }
        }
        if (adv){
            advSearch(input);
        }else{
            basicSearch(input);
        }
    }
    quit_search = TRUE;
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
    if (hash_init != 0){
        write(STDERR_FILENO, hash_error, strlen(hash_error));
		exit(1);
    }
    
    // Malloc scan_buffer
    scan_buffer_size = 10;
    scan_buffer = malloc(scan_buffer_size * sizeof(char *));
    for (i = 0; i < scan_buffer_size; ++i) {
        scan_buffer[i] = malloc(513 * sizeof(char *));
    }
    
	pthread_mutex_init(&mutex_one, NULL);
    pthread_mutex_init(&mutex_advanced, NULL);
    
    //TESTING PURPOSES ONLY (using old index.c)
    pthread_mutex_init(&mutex_test, NULL);
	
    pthread_cond_init(&empty_cond, NULL);
	pthread_cond_init(&fill_cond, NULL);
    
    pthread_cond_init(&indexed_cond, NULL);
    pthread_cond_init(&indexed_noticed_cond, NULL);
    
    finished_activations = 0;
    pthread_t index_threads[indexer_amount];
    createIndexThreads(index_threads, indexer_amount);
    
    pthread_t search_thread;
    pthread_create(&search_thread, NULL, startSearch, NULL);
    
    startScanning();
    
   
    for (i=0; i < indexer_amount; i++) {
        printf("JOIN %d?\nINDEX Thread %u\n\n",i,index_threads[i]);
        if(pthread_join(index_threads[i], NULL))
            printf("HERE's YOUR PROBLEM!\n");
    }
    
    indexing_done = TRUE;
    
    
    while(quit_search != TRUE){
        pthread_cond_signal(&indexed_cond);
        
    }
        
    pthread_join(search_thread, NULL);
    
	return 0;
}
