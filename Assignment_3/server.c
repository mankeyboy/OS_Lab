
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <signal.h>
#include <errno.h>

#define MSGSZ 4090
#define max_pids 100

int msqid;

typedef struct msgbuf 
{
    long    mtype;
    struct msg
    {
    	char    mtext[MSGSZ];
    	int    pid;
    }message;
}message_buf;

void sigint_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
    msgctl(msqid, IPC_RMID, NULL);
    exit(1);
}

void main()
{
   	void sigint_handler(int sig); /* prototype */
	char s[200];
	struct sigaction sa;

	sa.sa_handler = sigint_handler;
	sa.sa_flags = SA_RESTART; // or SA_RESTART
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	
   	int msgflg = IPC_CREAT | 0666;
   	key_t key;
   	message_buf sbuf,rbuf;
   	size_t buf_length=sizeof(struct msg);
   	int pid_arr[max_pids];
   	int pid_num = 0,j,k,l;
   	int sender_pid;
   	char    text[MSGSZ];

  

   key = 1234;


   if ((msqid = msgget(key, msgflg )) < 0) 
   {
        perror("msgget");
        exit(1);      
   }
   else 
	    printf("message queue created : %d\n",msqid);


  

while(1)
  {	


  	for(j=0;j<pid_num;j++)
  	{
       if(getpgid(pid_arr[j]) >= 0)
          continue;
       else
       {

         for(k=j;k<(pid_num-1);k++)
   		 {
   		    pid_arr[k] = pid_arr[k+1];
         }
              
         pid_arr[pid_num-1] = 0;
         printf("Currently coupled clients are\n");
         for(l=0;l<(pid_num-1);l++)
         {
     	   printf("Client PID : %d \n",pid_arr[j]);
         }
         pid_num--;	

       }
  	}

   printf("yoyo1\n");
   if (msgrcv(msqid, &rbuf, sizeof(struct msg), 1, 0) < 0) 
   {
   		printf("yoyo");
        perror("msgrcv");
        exit(1);
   }
   else printf("\n %d, %d, %s, %ld, %d:\n", msqid, rbuf.message.pid, rbuf.message.mtext, rbuf.mtype, (int)sizeof(struct msg));

   printf("xyz\n");
   sender_pid = rbuf.message.pid;

   if(strcmp(rbuf.message.mtext,"couple") == 0)
   {

    if( pid_num>0 )
    {
    	for(j=0;j<pid_num;j++)
    	{
    		if(pid_arr[j] == sender_pid)
    	 		break;
    	}	
    }
    else j = 0;
     
    if(j == pid_num)
    {
        pid_arr[pid_num] = rbuf.message.pid;
        pid_num++;
        printf("Currently coupled clients are\n");
        for(j=0;j<pid_num;j++)
        {
     	   printf("Client PID : %d \n",pid_arr[j]);
        }
     }	
     continue;
   	}
   
   if(strcmp(rbuf.message.mtext,"uncouple") == 0)
   {

   	  printf("\n got into this \n");
   	  for(j=0;j<pid_num;j++)
   	   {
   		 if (pid_arr[j] == rbuf.message.pid)
   		  {
   		     for(k=j;k<(pid_num-1);k++)
   		    	{
   		    	   pid_arr[k] = pid_arr[k+1];
                }
              
              pid_arr[pid_num - 1] = 0;
              
              
              printf("Currently coupled clients are\n");
              pid_num--;
              
              for(l=0;l<(pid_num);l++)
                {
     	           printf("Client PID : %d \n",pid_arr[j]);
                }
             	
              break;      
           } 
         
        }
       continue;
    }

     
    strcpy(text,rbuf.message.mtext); 
    for(j=0;j<pid_num;j++)
    {
        if(sender_pid == pid_arr[j])
           continue;

        else
        {    
    	    rbuf.mtype = pid_arr[j];
    	    char t[5]; 
    	    
    	    memset(rbuf.message.mtext,0,sizeof(rbuf.message.mtext));
    	    strcat(rbuf.message.mtext,"Terminal  ");
    	    sprintf(t, "%d",j);
            strcat(rbuf.message.mtext,t);
            strcat(rbuf.message.mtext,text);

    	    if (msgsnd(msqid, &rbuf, buf_length, IPC_NOWAIT) < 0) 
    	    {
       
               perror("msgsnd");
               exit(1);
       
            }

            else 
               printf("\n Message sent: %d, %d, %s, %ld, %d:\n", msqid, rbuf.message.pid, rbuf.message.mtext, rbuf.mtype, (int)sizeof(struct msg));
        }   

    }
  }
}


