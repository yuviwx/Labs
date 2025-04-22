#include <signal.h>
#include <stdio.h>
#include <string.h>
#include<unistd.h>


void signal_handler(int sig){
    printf("\nSignal %s was received\n", strsignal(sig)); // print the signal that was received
    signal(sig, SIG_DFL); // propagates the signal to the default signal handler
    raise(sig); // raise the signal so that the default signal handler can handle it - call the default signal handler.

    if(sig == 20) signal(SIGCONT, signal_handler);
    if(sig == 18) signal(SIGTSTP, signal_handler);
}

int main(int argc, char **argv){
    signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGCONT, signal_handler);
    
    while(1){
        sleep(2);
    }
    
    return 0;
}