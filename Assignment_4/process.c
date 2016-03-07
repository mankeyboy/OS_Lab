/*
13CS10043 S Raagapriya
13CS30021 Mayank Roy
*/

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/msg.h>


#define NOTIFY SIGINT
#define SUSPEND SIGTERM
#define IO SIGUSR1
#define TERMINATE SIGUSR2

//Main message queue type

typedef struct msgbuf 
{
  long    mtype;
  struct msg
  {
    int    prio;
    int    pid;
  }message;
}message_buf;


//global variables
message_buf sbuf;
int msqid;
int msgflg = IPC_CREAT | 0666;//access flags
key_t key;
pid_t   sched_pid;

//signal handler function
static void signal_handler(int sig, siginfo_t *siginfo, void *context)
{
	sched_pid=siginfo->si_pid;
	if(sig==NOTIFY)
	{
		//do nothing
		printf("Notified\n");
	}
	if(sig==SUSPEND)
	{
		printf("Suspending\n");
		pause();
	}
}

//function for sending message to msg queue
void send_msg()
{

  if (msgsnd(msqid, &sbuf, sizeof(struct msg), IPC_NOWAIT) < 0) 
  {
    perror("msgsnd");
    exit(1);
  }
  
  else 
    printf("Message Sent to \"%ld\"  \n", sbuf.mtype);
}


int main(int argc,char *argv[])
{
   
    pid_t pid=getpid();
    int i ,r;
    int prior;
    time_t t;
    
	sbuf.mtype = 1; 
	if(atoi(argv[4]) == 1)
	{	
	   sbuf.message.prio = 5;
	}
    else
	{
	   sbuf.message.prio = 10;
	}	
	sbuf.message.pid = getpid();
   
   
   // for the signals NOTIFY and SUSPEND 
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction=&signal_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(NOTIFY, &sa, NULL) < 0 || sigaction(SUSPEND, &sa, NULL)<0)
	{
		perror ("sigaction");
		return 0;
	}

   

   key = 1234;
   //creating msg queue
   if ((msqid = msgget(key, msgflg )) < 0) 
   {
        perror("msgget");
        exit(1);      
   }
   else 
       printf("joined mesage queue: %d\n",msqid);


   send_msg();
   //waiting for signal
   printf("Waiting for signal...\n");
   pause();//wait till a signal comes
   printf("Signal recieved\n");
   
   srand(atoi(argv[1]));
   //prior = (atoi(argv[4]));
   printf("itr = %s,prob = %s,sleep time = %s,prior = %s, %s\n",argv[1],argv[2],argv[3],argv[4],argv[5]);
   for(i=0;i<atoi(argv[1]);i++)
   {
	   	//printf("%d\n",i);
       // srand((unsigned) time(&t));
	   	r = rand()%100;
	    printf(" PID = %d loop_counter = %d\n",(int)getpid(),i);
	    if(r <= atoi(argv[2]))
	    {
	    	printf("i am sleeping\n");
	    	kill(sched_pid,IO);
	    	sleep(atoi(argv[3]));
	    	send_msg();
	    	pause();
			printf("IO complete...back to running\n");
	    }	

   }
   printf("i am done\n");
   kill(sched_pid,TERMINATE);
   return(0);	
}