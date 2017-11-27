//
// Created by yoav on 11/24/17.
//

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


int process_arglist(int count,char** arglist);
int checkForPipe(char** arglist);


void sigchld_handler(int signal){
    while (waitpid((pid_t) -1, 0, WNOHANG) > 0) {}
}



int process_arglist(int count,char** arglist){
    pid_t child_pid, firstChildPid;
    int pipeArgIndex, status, fds[2];
    bool isBackgroundProcess = false;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    if (strcmp(arglist[count - 1], "&") == 0) { // as arglist[count] is the NULL termination.
        isBackgroundProcess = true;
        arglist[count-1] = NULL; // in order not to pass "&" as an argument to execvp.
    }


    bool isPipe = (pipeArgIndex = checkForPipe(arglist)) != -1;
    if (isPipe) {
        arglist[pipeArgIndex] = NULL;
        pipe(fds);
    }

    for (int i=0; i < (1 + isPipe); i++){
        if ((child_pid = fork()) < 0){
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            exit(1);
        }

        if (child_pid == 0){
            // Save aside the pid of the 1st child, will use for waiting:
            if (i == 0) { firstChildPid = child_pid; }

            // Set appropriate SIGINT handler for child:
            sa.sa_handler = isBackgroundProcess ? SIG_IGN : SIG_DFL;
            if (sigaction(SIGINT, &sa, NULL) != 0){
                fprintf(stderr, "Signal registration failed: %s\n", strerror(errno));
                exit(1);
            }

            if (isPipe){
                // First process (i=0) will rplace stdout (1) with pipe and second (i=1) will replace stdin (0):
                dup2(fds[1-i], 1-i);
            }
            if (execvp(arglist[0 + i*(pipeArgIndex+1)],arglist + i*(pipeArgIndex +1)) == -1){
                fprintf(stderr, "execvp failed: %s\n", strerror(errno));
                exit(1);
            }
            break; //Child does not continue loop.
        }
    }

    if (child_pid != 0){
        if (isPipe){
            // Wait for both children (in the piped case):
            while (!isBackgroundProcess && (waitpid(child_pid,&status,0) != child_pid )
                   && (waitpid(firstChildPid, &status, 0) != firstChildPid)){
            }
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
    struct sigaction sigint_action;
    memset(&sigint_action, 0, sizeof(sigint_action));
    sigint_action.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &sigint_action, NULL) != 0){
        fprintf(stderr,"Signal registration failed: %s\n", strerror(errno));
        exit(1);
    }

    // Set father to reap zombies when they send SIGCHLD:
    struct sigaction sigchld_action;
    memset(&sigchld_action, 0, sizeof(sigchld_action));
    sigchld_action.sa_handler = &sigchld_handler;
    sigchld_action.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sigchld_action, NULL) != 0){
        fprintf(stderr,"Signal registration failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


int finalize(void) { return 0; }



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
