/*
13CS10043 S Raagapriya
13CS30021 Mayank Roy
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/shm.h>

using namespace std;

// Structs

//Main message queue type
typedef struct msgbuff 
{
  long    mtype;
  struct msg
  {
    int numb;
    pid_t pid;
  }message;
}message_buf;

// Global variables

message_buf mes;
int msqid1,msqid2;
int msgflg = IPC_CREAT | 0666;//access flags
key_t key1,key2;
struct msqid_ds *buf;
int q1[10],q2[10];
pid_t con[5],pro[5];
string s;
fstream f;
filebuf* inbuf;
char c;
key_t		key,q_key1,q_key2;			/* key to pass to semget() */
//int		semflg;			/* semflg to pass to semget() */
int		nsems;			/* nsems to pass to semget() */
int		semid,semid1,semid2;			/* return value from semget() */
struct  sembuf sop;
unsigned nsops;

// Function Definitions
void initialize();
void check_deadlock();
std::string get_working_path();

// Fuction Definitions

void sigint_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
  msgctl(msqid1, IPC_RMID, NULL);
  msgctl(msqid2, IPC_RMID, NULL);
  semctl(semid, 0, IPC_RMID, 0);
  semctl(semid1, 0, IPC_RMID, 0);
  semctl(semid2, 0, IPC_RMID, 0);
  exit(1);
}


std::string get_working_path()
{
   char temp[200];
   return ( getcwd(temp, 200) ? std::string( temp ) : std::string("") );
}

void initialize()
{
  int i;
  for(i=0;i<10;i++)
  {
    q1[i] = 0;
    q2[i] = 0;
  }	
}


void file_read()
{ 
  int i;
  f.open(s.c_str(), ios::in | ios::out );
  inbuf  = f.rdbuf();

  for(i=0;i<10;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q1[i] = atoi(&c);
  }

  c = inbuf->sbumpc();
  //printf("%c",c);

  for(i=0;i<10;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q2[i] = atoi(&c);
  }
  //printf("\n\n");
  f.close();
}

void check_deadlock()
{

  int i;
  int count = 0;
  while(1)
  {
    count++;
   
    sleep(2);
    printf("count %d\n",count);
    //sem_wait(sem_matrix);
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;
    semop(semid, &sop, 1); // requesting access for file
	  
    file_read();	 
     

    //deadlock checking
    int p1,p2; 
    p1 = -1;
    p2 = -1;
    for(i=0;i<5;i++)
    {
      if( (q1[i] == 1) && (q2[i] == 2) )
      {
        p1 = i;
      }
      
      if((q1[i] == 2) && (q2[i] == 1))
      {
        p2 = i;
      }	
    }	
  
    if((p1 != -1) && (p2 != -1))
    {
      printf("deadlock detected\n");
      printf("C%d(PID = %d) --> Q1 --> C%d(PID = %d) --> Q2 --> C%d(PID = %d)\n",p1,con[p1],p2,con[p2],p1,con[p1]);
      sleep(200);
      msgctl(msqid1, IPC_RMID, NULL);
      msgctl(msqid2, IPC_RMID, NULL);
      semctl(semid, 0, IPC_RMID, 0);
      semctl(semid1, 0, IPC_RMID, 0);
      semctl(semid2, 0, IPC_RMID, 0);
      killpg(0, SIGKILL);
    }
    sop.sem_op = 1;
    semop(semid, &sop, 1); // releasing file
  }
}

// MAIN FUNCTION

int main()
{
  void sigint_handler(int sig); /* prototype */
  struct sigaction sa;

  sa.sa_handler = sigint_handler;
  sa.sa_flags = SA_RESTART; // or SA_RESTART
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1) 
  {
    perror("sigaction");
    exit(1);
  }


  int n = 5, i;
  key1 = 1234;
  key2 = 1235;
  pid_t pid1,pid2;
  char *command;
  s = get_working_path() + "/matrix.txt";
  command = (char*)malloc(300*sizeof(char));

  //creating msg queue
  if ((msqid1 = msgget(key1, msgflg )) < 0) 
  {
    perror("msgget");
    exit(1);      
  }
  else 
    printf("message queue 1 created : %d\n",msqid1);

  if ((msqid2 = msgget(key2, msgflg )) < 0) 
  {
    perror("msgget");
    exit(1);      
  }
  else 
    printf("message queue 2 created : %d\n",msqid2);

  q_key1 = (key_t)20;
  nsems = 1;
  semid1 = semget(q_key1,nsems,IPC_CREAT|0666);
  if(semid1 == -1)
  {
    perror("semaphore 1 creation failed");
    exit(1);
  } 
  semctl(semid1, 0, SETVAL, 1);

  q_key2 = (key_t)30;
  nsems = 1;

  semid2 = semget(q_key2,nsems,IPC_CREAT|0666);
  if(semid2 == -1)
  {
    perror("semaphore 2 creation failed");
    exit(1);
  } 
  semctl(semid2, 0, SETVAL, 1);

  f.open(s.c_str(), ios::in | ios::out | ios::trunc);
  f<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"\n"
  <<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"\n"<<endl;
  f.close();

  initialize(); 

  key = (key_t)10;
  nsems = 1;

  semid = semget(key,nsems,IPC_CREAT|0666);
  if(semid == -1)
  {
    perror("semaphore creation failed");
    exit(1);
  }	
  semctl(semid, 0, SETVAL, 1);

  for(i=0;i<n;i++)
  {
    pid1 = fork();

    if(pid1 == 0)
    {
      printf("i am in child %d executing command\n",i);
      sprintf(command,"gnome-terminal -x sh -c './p %d; cat'",i);
      system(command);
      printf("after sys call %d\n",i);
      break;
    }
    else
    {	
      pro[i] = pid1;
      sleep(1);
    }  
  }
  
  if(pid1 != 0)
  {
    for(i=0;i<n;i++)
    {
      pid2 = fork();

      if(pid2 == 0)
      {
         printf("i am in child of consumer %d executing command\n",i);
         sprintf(command,"gnome-terminal -x sh -c './c %d; cat'",i);
         system(command);
         printf("after sys call of consumer %d\n",i);
         break;
      }
      else
      {   
        con[i] = pid2;
        sleep(1);
      }
    }
    if(pid2 != 0)
    {
      check_deadlock();
    }
  }
}