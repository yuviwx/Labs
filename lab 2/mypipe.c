#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char **argv) {
    int pipefd[2];
    char buf[6]; // 'hello' + '\0'
    
    if(pipe(pipefd) == -1){ // creating a pipe - creating a 'virtual file' were you can read and write from.
        perror("pipe");     // pipe return two file descriptors - [0] for reading and [1] for writing
        exit(1);
    }

    pid_t pid = fork(); // creates a child process - explanation below*
    if(pid == 0){        // child process - needs to send the parent a message through the pipe
        close(pipefd[0]); 
        if(write(pipefd[1], "hello", 6) == -1){
            perror("write didn't work");
            exit(1);
        }
        close(pipefd[1]);
    }
    else if(pid > 0){   // parent process - needs to read the message from the child, (explaination below**)
        close(pipefd[1]);
        if(read(pipefd[0], buf, 6) == -1) {
            perror("read didn't work");
            exit(1);
        }
        else {
            printf("%s\n", buf);
        }
        close(pipefd[0]);
        //waitpid(pid, NULL, 0); // explanation below***
    }
    else{
        perror("fork didn't work");
        exit(1);
    }
    
    return 0;
}
/* 
*  explanation 1: (for the fork and double close)
*  fork creates a child process that is an exact copy of the parent process.
*  The fork function duplicates the process image - meaning the complete memory representation the parent process.(code, data, stack, heap...)
*  Within the data being copy is the file descriptors, so when we called pipe and got 2 file descriptors, one for reading and one for writing,
*  now we have 4 file descriptors, where each 2 point to the same pipe, but we still need to close the file descriptor from both the parent and the child.
*  If we don't close the file descriptors, the pipe will not be closed and the read and write functions will not return. 
*  the file descriptors only close when the last reference to the pipe is closed.(like smart pointers in c++)

*  explanation 2: (for why we don't need to wait)
*  read and write are blocking functions, 
*  meaning that if the parent attempts to read before the child writes, 
*  the 'read' function will block the parent thread until data is available through the pipe.

*  explanation 3: (for the waitpid) - check!
*  waitpid is a function that waits for a child process to terminate.
*  the parent process will wait for the child process to terminate before it terminates itself.
*  if the parent process terminates before the child process, the child process will become a zombie process.
*  a zombie process is a process that has terminated but still has an entry in the process table.
*  the entry is needed so the parent can get the exit status of the child process.
*  the entry is removed when the parent process calls waitpid.
*  if the parent process doesn't call waitpid, the entry will remain in the process table until the system is rebooted.
*/