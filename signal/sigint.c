// Copyright [2020] <Puchkov Kyryll>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sighandler(int);

int main(void) {
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    // Get the proccess id
    pid_t pid = getpid();
    while (1) {
        printf("Proccess with pid %d to sleep for a second...\n", pid);
        sleep(1);
    }
    return(0);
}

void sighandler(int signum) {
    const char* signal_number;
    switch (signum) {
        case 2:
            signal_number = "SIGINT";
            break;
        case 15:
            signal_number = "SIGTERM";
            break;
        default:
            signal_number = "Did not know yet!";
    }

    printf("Caught signal %d: %s\n", signum, signal_number);
    exit(1);
}
