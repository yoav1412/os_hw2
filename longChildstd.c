
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char ** argv){

	int times;
	scanf("%d", &times);

	while(times > 0){
		printf("In longChild (%d)\n",times);
		times--;
		sleep(1);
	}


}