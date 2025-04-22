// This file implement pipe behavior for a simple command - 'ls -l | tail -n 2'
// Pipe is used to combine two or more commands, and in this, the output of one command acts as the input of the next command.

#include <stdlib.h>
#include <unistd.h>   // For pipe() fork() close()
#include <sys/types.h> // For pid_t type
#include <sys/wait.h> // For waitpid()
#include <stdio.h>    // For perror()

// Declarations
pid_t first_process_ls(int pipefd[]);
void second_process_tail(int pipefd[], pid_t pid);


int main(int argc, char **argv) {
    // 1. Create a pipe:
    int pipefd[2]; 
    if(pipe(pipefd) == -1){ 
        perror("pipe");     
        return 1;
    } 

    // The first child output 'ls -l' to the pipe input(his stdout)
    // The second child run 'tail -n 2' - input from pipe(his stdin) and output(regular stdout)  
    pid_t pid = first_process_ls(pipefd);
    second_process_tail(pipefd, pid);

    return 0;
}

// 1. Forks a new child
// 2. Redirect stdout to write-end of pipe
// 3. Execute "ls -l" 
pid_t first_process_ls(int pipefd[]){
    perror("parent_process>forking…");
    pid_t pid = fork();
    if(pid<0){ // in parent if fork failed
      perror("fork fail");
      exit(1);
    }

    if(pid == 0){ // Child process: writes to stdout(pipe's)
        close(pipefd[0]); // unused

        perror("child1>redirecting stdout to the write end of the pipe…");
        close(1);
        dup(pipefd[1]);
        close(pipefd[1]);
        
        perror("child1>going to execute cmd: ls -l");
        char *args[] = {"ls", "-l", NULL};
        execvp("ls", args);
        perror("execvp failed");
        exit(1);
    }

    if(pid > 0){ // Parent process:
        fprintf(stderr ,"parent_process>created process with id: %d\n", pid);
        fprintf(stderr, "parent_process>closing the write end of the pipe…\n");
        close(pipefd[1]);
    }

    return pid;
}

// 1. Forks a new child
// 2. Redirect stdin to read-end of pipe
// 3. Execute "tail -n 2" 
void second_process_tail(int pipefd[], pid_t pid){
    
    // 5.Fork a second child process:
    perror("parent_process>forking…");
    pid_t pid2 = fork();
    if(pid2<0){
      perror("fork fail");
      exit(1);
    }

    // 6. In child process: explanation below**
    if(pid2 == 0){        
        perror("child2>redirecting stdin to the read end of the pipe…");
        close(0);
        dup(pipefd[0]);
        close(pipefd[0]);
        //close(pipefd[1]); // unused


        perror("child2>going to execute cmd: tail -n 2");
        char* args[] = {"tail", "-n", "2", NULL};
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }

    // 7. In parent process:
    if(pid2 > 0){
        fprintf(stderr ,"parent_process>created process with id: %d\n", pid2);
        fprintf(stderr, "parent_process>closing the read end of the pipe…\n");
        close(pipefd[0]);
        fprintf(stderr, "parent_process>waiting for child processes to terminate…\n");
        waitpid(pid, NULL, 0);
        waitpid(pid2, NULL, 0);
    }
}

/* explanation*:
3. In the child1 process:
3.1. Close the standard output. - fd == 1
3.2. Duplicate the write-end of the pipe using dup (see man).
3.3. Close the file descriptor that was duplicated.
3.4. Execute "ls -l".
*/

/* explanation**:
6. In the child2 process:
6.1. Close the standard input.
6.2. Duplicate the read-end of the pipe using dup.
6.3. Close the file descriptor that was duplicated.
6.4. Execute "tail -n 2".
*/