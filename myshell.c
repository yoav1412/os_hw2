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
//TODO: handle zombies...
//TODO: go back from SIG_DFL and SIG_IGN to handles?
//TODO: prepare and finalize .....
//TODO: FORUM: vulnarability? in the "|" case, father process spawns 2 children. he sets his handler to ignoe SIGINT only after both are spawned (otherwise, the children too will not stop). is it ok? (possibly there could be a sigint between spawnings...)

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

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));


    bool isBackgroundProcess = false;
    if (strcmp(arglist[count - 1], "&") == 0) { // as arglist[count] is the NULL termination.
        isBackgroundProcess = true;
        arglist[count-1] = NULL; // in order not to pass "&" as an argument to execvp.
    }

    int pipeArgIndex, firstChildPid;
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

            printf("In the child process.\n"); // TODO: rm

            if (i == 0) { firstChildPid = child_pid; } // Save aside the pid of the 1st child, will use for waiting.

            // Set appropriate SIGINT handler for child:
            sa.sa_handler = isBackgroundProcess ? SIG_IGN : SIG_DFL;
            if (sigaction(SIGINT, &sa, NULL) != 0){
                printf("Signal registration failed: %s\n", strerror(errno));
                exit(1);
            }


            if (isPipe){
                // First process (i=0) will rplace stdout (1) with pipe and second (i=1) will replace stdin (0):
                close(1-i);
                dup(fds[1-i]);
            }

            if (execvp(arglist[0 + i*(pipeArgIndex+1)],arglist + i*(pipeArgIndex +1)) == -1){
                printf("execvp failed: %s\n", strerror(errno));
                exit(1);

            }
            break; //Child does not continue loop.
        }
    }



    if (child_pid != 0){



        if (isPipe){
            // Wait for both children (in the piped case):
            while (!isBackgroundProcess && (wait(&status) != child_pid) && (wait(&status) != firstChildPid)){}
        }
        else {
            // Wait for child (non-piped case):
            while (!isBackgroundProcess && (wait(&status) != child_pid) ){}
        }
        

    }
    return 1;

}


int prepare(void){
    // Set father to ignore SIGINT while son is running:
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &sa, NULL) != 0){
        printf("Signal registration failed: %s\n", strerror(errno));
        exit(1);
    }

    return 0;}


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