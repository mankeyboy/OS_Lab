/*

13CS30021
Mayank Roy
OS Lab Test 1
Machine Number 89

*/

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <bits/stdc++.h>
#include <sys/ipc.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>

using namespace std;

int min_value, min_value_seen;

void sigend_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
    exit(min_value);
}

void sigstart_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
    
}

int main(int argc, char* argv[])
{
    void sigend_handler(int sig); /* prototype */
    struct sigaction sa_end;

    sa_end.sa_handler = sigend_handler;
    sa_end.sa_flags = SA_RESTART; // or SA_RESTART
    sigemptyset(&sa_end.sa_mask);

    if (sigaction(SIGUSR2, &sa_end, NULL) == -1) 
    {
        perror("sigaction end");
        exit(1);
    }
    void sigstart_handler(int sig); /* prototype */
    struct sigaction sa_start;

    sa_start.sa_handler = sigstart_handler;
    sa_start.sa_flags = SA_RESTART; // or SA_RESTART
    sigemptyset(&sa_start.sa_mask);

    if (sigaction(SIGUSR1, &sa_start, NULL) == -1) 
    {
        perror("sigaction start");
        exit(1);
    }
    if(argc < 2 )
    {
    printf("Invalid number of arguments\n");
    return 0 ;
    }
    min_value = min_value_seen = 0;

    min_value = atoi(argv[1]);
    min_value_seen = min_value;
    return 0;
}