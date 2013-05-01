
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


// Words to be indexed along with metadata
Container pending_words[10];

char argc_error[83] = "Invalid amount of args!\nUsage: search-engine [indexer_thread_amount] [file-list]\n\n\0";
char unable_to_open[34] = "Invalid file or unable to open!\n\n\0";



int main(int argc, char * argv[]){

	// Let's try to guarantee proper usage
	if(argc != 3){
		write(STDERR_FILENO, argc_error, strlen(argc_error));
		exit(1);
	}

	FILE * file_list;
	file_list = fopen(argv[2],"r");
	
	// If file is invalid or isn't loaded for some reason, there is no point of moving on
	if(file_list == NULL){
		write(STDERR_FILENO, unable_to_open, strlen(unable_to_open));
		exit(1);
	}
	
	// How many threads will be indexing
	int indexer_amount = atoi(argv[1]);

	//BUFFER SIZE??????????????
	char buffer[512];
	int line_number;
	char * word;
	char * save_ptr;

	// Read in file names from the list of files a.k.a. file_list
	while(fgets(buffer, 512, file_list) != NULL){
		printf("%s",buffer);
	}

	fclose(file_list);

	return 0;
}
