#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#define MAX_SIZE 200

int main(int argc, char * argv[]) {
	openlog(NULL, LOG_PID, LOG_USER);
	if(argc != 3) {
	      	syslog(LOG_ERR, "No enough variables.\n");	
		return 1; 
	}
	char * writefile = (char*)malloc(sizeof(char)*MAX_SIZE);
	char * writestr = (char*)malloc(sizeof(char)*MAX_SIZE);
	writefile = argv[1];
	writestr = argv[2];
	FILE * fp = fopen(writefile, "w+");
	if(fp == NULL) {
	       	syslog(LOG_ERR, "Unable to write the file.\n");	
		return 1; 
	}
	syslog(LOG_DEBUG, "Write %s to %s\n", writestr, writefile);
	fputs(writestr, fp);
	fclose(fp);
	closelog();

	return 0;
}
