//
// Created by yoav on 11/24/17.
//

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "myshell.h"

//TODO: what's SIGCHILD? need to use it

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
    if (strcmp(arglist[count - 1], "&") == 0) { // as arglist[count] is the NULL termination.
        isBackgroundProcess = true;
        arglist[count-1] = NULL; // in order not to pass "&" as an argument to execvp.
    }

    int pipeArgIndex;
    int fds[2];
    bool isPipe = (pipeArgIndex = checkForPipe(arglist)) != -1;
    if (isPipe) {
        arglist[pipeArgIndex] = NULL;
        pipe(fds);
    }

    for (int i=0; i < 1 + isPipe; i++){
        if ((child_pid = fork()) < 0){
            printf("fork failed: %s\n", strerror(errno));
            exit(1);
        }

        if (child_pid == 0){
            // Child code:

            signal(SIGINT, child_handler); //TODO: change to sigaction
            //printf("In the child process\n"); // TODO: rm

            if (isPipe){
                // First process (i=0) will rplace stdout (1) with pipe and second (i=1) will replace stdin (0):
                close(1-i);
                dup(fds[1-i]);
            }

            if (execvp(arglist[0 + i*(pipeArgIndex+1)],arglist + i*(pipeArgIndex +1)) == -1){
                printf("execvp failed: %s\n", strerror(errno));

            }
            break; //Child does not continue loop.
        }
        else{
            signal(SIGINT, parent_handler); //TODO: change to sigaction
            }
    }

    if (child_pid != 0){
        while (!isBackgroundProcess && wait(&status) != -1){} // Wait for child / children
        printf("In fater. finished waiting for son(s).\n"); // TODO: rm
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