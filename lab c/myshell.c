#define _GNU_SOURCE
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

typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;

// For status:
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0
// For history:
#define HISTLEN 10 // it should be the history current size???

typedef struct myqueue
{
    int head;
    int tail;
    int size;
    char** queue;
    void (*enquque)(char*);
    void (*dequeue)();
    void (*print)();
    int (*isEmpty)();
    void (*redo)(cmdLine*, int, int, process**);
} myqueue;

myqueue* history;

void execute(cmdLine *pCmdLine, int debug, process** process_list); // for none pipe command - uses fork
void handleExecute(cmdLine *cmd, int debug, pid_t pid, process** process_list); // 
void handleCmd(cmdLine* cmd, int debug, process** process_list);
void handleSignal(cmdLine* cmd);

// Pipe's functions
void handlePipeCmd(cmdLine *cmd, int debug, process** process_list); // handle multiply fork and redirection of command, calls 'handleExecute' in the end.
pid_t runPipeLeft(cmdLine *cmd, int debug, int pipefd[], process** process_list);
void runPipeRight(cmdLine *cmd, int debug, int pipefd[], pid_t pid, process** process_list);

// Process's functions
void addProcess(process** process_list, cmdLine* cmd, pid_t pid); 
void printProcessList(process** process_list);
void freeProcessList(process* process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);
// cmdLine* deepCopyCmdLine(cmdLine* cmd);

// History's functions
void history_constructor();
void addToHistory(char* cmd);
void printHistory();
int isHistoryEmpty();
void redo_cmd(cmdLine* cmd, int num, int debug, process** process_list);

int main(int argc, char **argv) {
    char cwd[PATH_MAX];
    char buffer[2048];
    cmdLine* cmd;
    int debug = 0; // off

    // Initialize history
    history_constructor();


    // Initialize process list
    process* process_head = NULL;
    process** process_list = &process_head;

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

        addToHistory(buffer);
        cmd = parseCmdLines(buffer);
        handleCmd(cmd,debug,process_list);  
        //free(cmd); // we won't use free cmd lines because it'll free the pointer inside as well and we need them for the process list.
        //freeCmdLines(cmd);
    }
}

// add proc
void handleCmd(cmdLine* cmd, int debug, process** process_list){

    // Handle the command
    if(cmd != NULL){ // 'parseCmdLines' returns NULL if unsuccessful 

        // Check if the command is empty - edge case
        if(cmd->argCount == 0){
            freeCmdLines(cmd);
            return;
        }

        if(cmd->arguments[0][0] == '!'){

            if (strcmp(cmd->arguments[0], "!!") == 0) {
            history->redo(cmd, 1, debug, process_list);
            return;
            }
            
            if(strlen(cmd->arguments[0]) < 2){
                perror("unvalid command");
                return;
            }
            int num = atoi(&cmd->arguments[0][1]);

            if(num == 0){
            perror("unvalid command");
            return;
            }
            
            history->redo(cmd, num, debug, process_list);
            return;
        }

        // quit command
        if(strcmp(cmd->arguments[0], "quit") == 0){
            freeProcessList(*process_list);
            freeCmdLines(cmd);
            exit(0);
        }

        if(strcmp(cmd->arguments[0], "procs") == 0){
            printProcessList(process_list);
            return;
        }

        if(strcmp(cmd->arguments[0], "history") == 0){
            history->print();
            return;
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

        // wake\stop\term command
        else if(strcmp(cmd->arguments[0], "wake") == 0 || strcmp(cmd->arguments[0], "stop") == 0 || strcmp(cmd->arguments[0], "term") == 0){
            handleSignal(cmd); 
        }

        // pipe command
        else if(cmd->next){
            handlePipeCmd(cmd, debug, process_list);
        }

        // execute other commands
        else{execute(cmd, debug, process_list);}
    }
}

void handleSignal(cmdLine* cmd){
    // Handle the signal
    int sig = strcmp(cmd-> arguments[0], "stop") == 0 ? SIGSTOP : strcmp(cmd-> arguments[0], "wake") == 0 ? SIGCONT : SIGINT; 
    int pid = atoi(cmd->arguments[1]);
    if(kill(pid, sig) == -1) {perror("failed to send signal");}
}

// seperate between regular command and pipe's command because pipe already forked and redirected i/o.
void execute(cmdLine *cmd, int debug, process** process_list){
    pid_t pid = fork(); // create a new process
    if(pid < 0){
        perror("fork failed");
        exit(1);
    }
    handleExecute(cmd,debug,pid, process_list);    
}

// Execute
void handleExecute(cmdLine *cmd, int debug, pid_t pid, process** process_list){
    
    if(pid > 0){ // parent process
        addProcess(process_list, cmd, pid);
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

// Pipe:

// This function will handle 2 pipe of 2 commands
void handlePipeCmd(cmdLine *cmd, int debug, process** process_list){
   
    // Edge cases:
    if(cmd->outputRedirect || cmd->next->inputRedirect){
        perror("redirection of pipe's first command output or pipe's second command input Lacks any logic");
        return;
    }

    // Create pipe
    int pipefd[2]; 
    if(pipe(pipefd) == -1){ 
        perror("pipe");     
        return;
    } 

    // Execute
    pid_t pid = runPipeLeft(cmd, debug, pipefd, process_list);
    runPipeRight(cmd, debug, pipefd, pid, process_list);
}

// 1. Fork a new child
// 2. Redirect the output to the pipe input
// 3. Execute the command
pid_t runPipeLeft(cmdLine *cmd, int debug, int pipefd[], process** process_list){
    // 1.
    pid_t pid = fork();
    if(pid < 0){
        perror("fork fail");
        exit(1);
    }

    if(pid > 0) fprintf(stderr ,"parent_process>created process for left command with id: %d\n", pid);

    // 2. + 3.
    if(pid == 0){
        close(STDOUT_FILENO);
        dup(pipefd[1]); // pipefd[1] - pipe's input
        close(pipefd[1]);
        close(pipefd[0]); // unused

        handleExecute(cmd, debug, pid, process_list);
    }

    if(pid > 0){
        fprintf(stderr, "parent_process>closing the write end of the pipe…\n");
        close(pipefd[1]);
    }

    return pid;
}

// 1. Fork a new child
// 2. Redirect the input to the pipe output
// 3. Execute the command
void runPipeRight(cmdLine *cmd, int debug, int pipefd[], pid_t pid, process** process_list){
    // 1.
    pid_t pid2 = fork();
    if(pid2 < 0){
        perror("fork fail");
        exit(1);
    }

    if(pid2 > 0) fprintf(stderr ,"parent_process>created a process for right command with id: %d\n", pid);

    // 2. + 3.
    if(pid2 == 0){
        close(STDIN_FILENO);
        dup(pipefd[0]); // pipefd[0] - pipe's output
        close(pipefd[0]);

        handleExecute(cmd, debug, pid2, process_list);
    }

    if(pid2 > 0){
        fprintf(stderr, "parent_process>closing the write end of the pipe…\n");
        close(pipefd[0]);
        fprintf(stderr, "parent_process>waiting for child processes to terminate…\n");
        waitpid(pid, NULL, 0);
        waitpid(pid2, NULL, 0);
    }
}

// Process:

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = (process*)malloc(sizeof(process));
    if(!new_process){
        perror("malloc failed");
        exit(1);
    }
    // Deep copy of cmdLine
    new_process->cmd = (cmdLine*)malloc(sizeof(cmdLine));
    if (!new_process->cmd) {
        perror("malloc failed");
        free(new_process);
        exit(1);
    }

    // Copy the main structure
    memcpy(new_process->cmd, cmd, sizeof(cmdLine));
    //new_process->cmd = parseCmdLines(cmd->arguments[0]);
     for (int i = 1; i < cmd->argCount; i++) {
        replaceCmdArg(new_process->cmd, i, cmd->arguments[i]);
    }

    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

void printProcessList(process** process_list){
    updateProcessList(process_list);

    char* status[] = {"TERMINATED", "SUSPENDED", "RUNNING"};
    process* curr = *process_list;
    process* prev = NULL; // for deleting in a linked list

    printf("PID\tCommand\t\tSTATUS\n");
    while(curr != NULL){
        printf("%d\t%s\t\t%s\n", curr->pid, curr->cmd->arguments[0], status[(curr->status)+1]);
        
        // Delete 'freshly' terminated processes
        if(curr->status == TERMINATED){
            if(!prev) {
                (*process_list) = curr->next; // new head
            } else{
                prev->next = curr->next;
            }
            freeCmdLines(curr->cmd);
            free(curr);
            curr = prev ? prev->next : *process_list;
        }
        else{
            prev = curr;
            curr = curr->next;
        }
    }
}

void freeProcessList(process* process_list){
    process* temp;
    while (process_list) {
        temp = process_list;
        process_list = process_list->next;
        freeCmdLines(temp->cmd); 
        free(temp);             
    }
}

// update the status of the process -  Do not remove processes!
// Remove process only happens after printing(as seen in the example).
void updateProcessList(process **process_list){
    process* curr = *process_list;
    int status;
    pid_t result;
    //printf("%s", (*process_list)->cmd->arguments[0]);
    while(curr){
        result = waitpid(curr->pid,&status,WNOHANG | WUNTRACED | WCONTINUED);

        if(result < 0){ 
            curr->status = TERMINATED;
        }

        else if(result != 0) { // 0 means nothing changed
            if(WIFEXITED(status) || WIFSIGNALED(status))  // terminated
                curr->status = TERMINATED;
            
            else if(WIFSTOPPED(status))                   // stopped
                curr->status = SUSPENDED;
            
            else if(WIFCONTINUED(status))                 // continue running
                curr->status = RUNNING;
        }

        curr = curr->next;
    }
}

// find the process with the given id in the process_list and change its status to the received status.
void updateProcessStatus(process* process_list, int pid, int status){
    process* curr = process_list;

    while(curr){
        if(curr->pid == pid){
            curr->status = status;
            return;
        }
        curr = curr->next;
    }
}

void history_constructor(){
    history = malloc(sizeof(myqueue));
    history->head = -1;
    history->tail = -1;
    history->size = 0;
    history->queue = (char**)calloc(HISTLEN, sizeof(char*));

    history->enquque = &addToHistory;
    history->dequeue = NULL;
    history->print = &printHistory;
    history->isEmpty = &isHistoryEmpty;
    history->redo = &redo_cmd;

    for(int i = 0; i < 10; i++){
        history->queue[i] = (char*)malloc(2048);
    }
}

void addToHistory(char* cmd){
    int head = history->head;
    int tail = history->tail;
    char** queue = history->queue;

    // The queue is empty
    if(head == -1){
        queue[0] = (char*)malloc(2048); // 2048*1 = 2048 where sizeof(char) == 1
        head = 0;
        tail = 0;
        strcpy(queue[tail], cmd);
    }

    // The queue isn't empty
    else{ 
        tail = (tail + 1) % 10; // in a queue you enter the end of it
        strcpy(queue[tail], cmd);
        if(tail == head){
            head = (head + 1) % 10;
        }
    }

    if(tail >= head) history->size++;
    history->head = head;
    history->tail = tail;
}

void printHistory(){
    printf("entry\tcommand\n");

    // empty
    if(history->isEmpty()) return;

    int curr = history->head;
    int tail = history->tail;
    char** queue = history->queue;
    int entry = 1;

    do{
       printf("%d\t%s", entry++, queue[curr]); 
       curr = (curr + 1) % 10;
    }while(curr != (tail+1)%10);
}

int isHistoryEmpty(){
    return history->size == 0;
}

void redo_cmd(cmdLine* cmd, int num, int debug, process** process_list){
    if(history->isEmpty()) return;
    
    if(num > history->size){
        fprintf(stderr,"Only %d commands in history\n", history->size);
        return;
    }

    int prev_int = (history->head + num - 1) % 10; // insure no negative indexes
    freeCmdLines(cmd);
    char* prev_cmd = history->queue[prev_int];
    cmd = parseCmdLines(prev_cmd);
    history->enquque(prev_cmd); 
    handleCmd(cmd, debug, process_list);
}