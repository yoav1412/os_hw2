//
// Created by yoav on 11/24/17.
//

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "myshell.h"


void parent_handler(int signal){
    printf("Parent caught signal. ignoring\n"); //TODO rm
    return;
}

void child_handler(int signal){
    printf("Child caught signal. exiting..\n"); //TODO rm
    exit(signal);
}


int process_arglist(int count,char** arglist){
    pid_t child_pid;
    int status;

    bool isBackgroundProcess = false;
    if (strcmp(arglist[count - 1], "&") == 0) { // as argist[count] is the NULL termination.
        isBackgroundProcess = true;
        arglist[count-1] = NULL; // in order not to pass "&" as an argument to execvp.
    }

    int pipeArgIndex;
    bool isPipe = (pipeArgIndex = checkForPipe(arglist)) != -1;
    if (isPipe) {
        arglist[pipeArgIndex] = NULL;
    }


    for (int i=0; i < 1 + isPipe; i++){
        if ((child_pid = fork()) < 0){
            printf("fork failed: %s\n", strerror(errno));
            exit(1);
        }

        if (child_pid == 0){
            // Child code:
            signal(SIGINT, child_handler);
            printf("In the child process\n"); // TODO: rm
            if (execvp(arglist[0 + i*(pipeArgIndex+1)],arglist + i*(pipeArgIndex +1)) == -1){
                printf("execvp failed: %s\n", strerror(errno));

            }
            printf("After execvp\n"); // TODO: rm
        }
        else {
            signal(SIGINT, parent_handler);
            while (!isBackgroundProcess && wait(&status) != child_pid){} // Wait for the child to finish.
            printf("In fater. finished waiting for son.\n"); // TODO: rm
        }
    }


    //TODO: need to restore father process SIGINT handler here. (not default??..)

    return 1;


}


int prepare(void){ return 0;}
int finalize(void) {return 0;}



int checkForPipe(char** arglist){
    int i=0;
    while (arglist[i] != NULL){
        char* arg = arglist[i];
        if (0 == strcmp(arg,"|")){
            return i;
        }
        i++;
    }
    return -1;
}