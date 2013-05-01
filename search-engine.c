
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "index.h"



typedef struct container{
	char file_name[512];
	int f_size;
	char word[512];
	int w_size;
	int line;
}container;


char argc_error[83] = "Invalid amount of args!\nUsage: search-engine [indexer_thread_amount] [file-list]\n\n\0";
char unable_to_open[34] = "Invalid file or unable to open!\n\n\0";



int main(int argc, char * argv[]){

	if(argc != 3){
		write(STDERR_FILENO, argc_error, strlen(argc_error));
		exit(1);
	}

	FILE * file_list;
	file_list = fopen(argv[2],"r");
	
	if(file_list == NULL){
		write(STDERR_FILENO, unable_to_open, strlen(unable_to_open));
		exit(1);
	}
	
	int thread_number = atoi(argv[1]);

	return 0;
}
