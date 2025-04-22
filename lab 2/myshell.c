#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>


void execute(cmdLine *pCmdLine, int debug);
void handleCmd(cmdLine* cmd, int debug);
void handleSignal(cmdLine* cmd);

int main(int argc, char **argv) {
    char cwd[PATH_MAX];
    char buffer[2048];
    cmdLine* cmd;
    int debug = 0; // off

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-d") == 0) debug = 1;
    }

    while(1){

        // Display a prompt
        if((getcwd(cwd, PATH_MAX)) == NULL){ //unsuccessful call shall return  a null pointer
            perror("getcwd() error");
            return 1; // calls exit with the parameter 1 - meaning 'Exit Failure'
        }
        printf("%s$ ", cwd);

        // Read a line from the "user"
        if(fgets(buffer, 2048, stdin) == NULL){
            perror("fgets() error");
            return 1;
        }
        
        cmd = parseCmdLines(buffer);
        handleCmd(cmd,debug);  
        freeCmdLines(cmd);  
    }
}

void handleCmd(cmdLine* cmd, int debug){

    // Handle the command
    if(cmd != NULL){ // 'parseCmdLines' returns NULL if unsuccessful 
        
        // Check if the command is empty - edge case
        if(cmd->argCount == 0){
            freeCmdLines(cmd);
            return;
        }

        // quit command
        if(strcmp(cmd->arguments[0], "quit") == 0){
            freeCmdLines(cmd);
            exit(0);
        }

        // check vaildity of the command
        if(cmd->argCount == 1 && (strcmp(cmd->arguments[0], "cd") == 0 || strcmp(cmd->arguments[0], "wake") == 0 
                                || strcmp(cmd->arguments[0], "stop") == 0 || strcmp(cmd->arguments[0], "term") == 0)){
            perror("missing argument");
            return;
        }

        // cd command
        if(strcmp(cmd->arguments[0], "cd") == 0){
            if(chdir(cmd->arguments[1]) == -1) fprintf(stderr, "cd: %s: No such file or directory\n", cmd->arguments[1]);
        }

        // stop command
        else if(strcmp(cmd->arguments[0], "wake") == 0 || strcmp(cmd->arguments[0], "stop") == 0 || strcmp(cmd->arguments[0], "term") == 0){
            handleSignal(cmd); 
        }

        // execute other commands
        else{execute(cmd, debug);}
    }
}

void handleSignal(cmdLine* cmd){
    // Handle the signal
    int sig = strcmp(cmd-> arguments[0], "stop") == 0 ? SIGSTOP : strcmp(cmd-> arguments[0], "wake") == 0 ? SIGCONT : SIGINT; 
    int pid = atoi(cmd->arguments[1]);
    if(kill(pid, sig) == -1) perror("failed to send signal");
}

void execute(cmdLine *cmd, int debug){
    pid_t pid = fork(); // create a new process

    if(pid > 0){ // perent process
        if(debug) 
            fprintf(stderr, "Child PID: %d\nExecuting command: %s\n", pid, cmd->arguments[0]);
        if(cmd->blocking) 
            waitpid(pid, NULL, 0);
    }

    else if(pid == 0){ // child process
        
        // Handle input redirection
        if (cmd->inputRedirect) { // NULL is no inputRedirect (NULL == false)
            close(0); // Close standard input (fd 0)
            if (open(cmd->inputRedirect, O_RDONLY) < 0) {
                perror("Input redirection failed");
                _exit(1);
            }
        }

        // Handle output redirection
        if (cmd->outputRedirect) {
            close(STDOUT_FILENO); // Close standard output
            if (open(cmd->outputRedirect, O_WRONLY | O_CREAT | O_APPEND, 0666) < 0) { // 0666 - everyone can read and write
                perror("Output redirection failed");
                _exit(1);
            }
        }

        execvp(cmd->arguments[0], cmd->arguments);
        perror("An error as occured during the execution of the command");
        _exit(1);
    }
    // ignore the case of pid < 0 - fork bomb
}
