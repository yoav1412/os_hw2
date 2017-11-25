//
// Created by yoav on 11/24/17.
//

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "myshell.h"


int process_arglist(int count,char** arglist){
    pid_t child_pid;
    int status;
    if ((child_pid = fork()) < 0){
        printf("fork failed: %s\n", strerror(errno));
        exit(1);
    }
    bool isBackgroundProcess = false;
    if (strcmp(arglist[count - 1], "&") == 0) { // as argist[count] is the NULL termination.
        isBackgroundProcess = true;
        arglist[count-1] = NULL; // in order not to pass "&" as an argument to execvp.
    }
    //printf("lastArg = %c",lastArg);

    if (child_pid == 0){
        // Child code:
        printf("In the child process\n"); // TODO: rm
        if (execvp(arglist[0],arglist) == -1){
            printf("execvp failed: %s\n", strerror(errno));

        }
        printf("After execvp\n"); // TODO: rm
    }
    else {
        while (!isBackgroundProcess && wait(&status) != child_pid){} // Wait for the child to finish.
        printf("In fater. finished waiting for son.\n"); // TODO: rm
    }


    return 1;


}


int prepare(void){ return 0;}
int finalize(void) {return 0;}