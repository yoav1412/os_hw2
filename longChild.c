
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char ** argv){

	int times = atoi(argv[1]);
	char name[100];
	if (argc == 3){
		strcpy(name ,argv[2]);
		}
	while(times > 0){
		printf("In longChild(%s)->(%d)\n",name,times);
		times--;
		sleep(1);
	}


}