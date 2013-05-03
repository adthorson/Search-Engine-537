#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "index.h"



typedef struct Container{
	char file_name[512];
	int f_size;
	char word[512];
	int w_size;
	int line;
}Container;


Container pending_words[10];    // Words to be indexed along with metadata
char * scan_buffer[10000];      // Producer/consumer queue
int scan_count = 0;

char argc_error[83] = "Invalid amount of args!\nUsage: search-engine [indexer_thread_amount] [file-list]\n\n\0";
char unable_to_open[34] = "Invalid file or unable to open!\n\n\0";


add_to_buffer(char * filename) {
    scan_buffer[scan_count] = malloc((strlen(filename)));
    scan_buffer[scan_count] = filename;
    printf("%s",scan_buffer[scan_count]);
    ++scan_count;
}

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
 */
activate_Multi_Threader(int number_of_threads, void function_name, struct parameter){
	pthread_t tidp;
	int i;
	for(i = 0; i < number_of_threads; i++){
	//int  pthread_create(pthread_t  *  thread, pthread_attr_t * attr, void *(*start_routine)(void *), void * arg);
		thread0 = pthread_create(&tidp, NULL, &function_name, (void *) &parameter);
		if(thread0 != 0) printf("\ncan't create thread\n");
	}
	
	pthread_join(tidp, NULL); 

}

//NOTE NOTE NOTE
//in order to pass in a method to a threader, YOU NEED TO name your method in this manner
//add_to_buffer(char * filename)  needs to be
//void* add_to_buffer(char * filename)


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

	//BUFFER SIZE?
	char buffer[512];
	int line_number;
	char * word;
	char * save_ptr;

    // SCANNER
	// Read in file names from the list of files a.k.a. file_list
	while(fgets(buffer, 512, file_list) != NULL) {
        add_to_buffer(buffer);
	}
    
    // INDEXER
    

	fclose(file_list);

	return 0;
}
