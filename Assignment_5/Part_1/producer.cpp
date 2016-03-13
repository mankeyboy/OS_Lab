/*
13CS10043 S Raagapriya
13CS30021 Mayank Roy
*/

#include <iostream>
#include <fstream>
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

using namespace std;

// Main message queue type
typedef struct msgbuff 
{
  long    mtype;
  struct msg
  {
    int numb;
    int pid;
  }messag;
}message_buf;

struct msg
  {
    int numb;
    int pid;
  }message;

// Global variables
message_buf mes;
int msqid1,msqid2;
int msgflg = IPC_CREAT | 0666;//access flags
key_t key1,key2;
struct msqid_ds buf;
int q1[10],q2[10];
string s;
fstream f;
filebuf* inbuf;
filebuf* outbuf;
char c;
key_t		key,q_key1,q_key2;			/* key to pass to semget() */
int		nsems;			/* nsems to pass to semget() */
int		semid,semid1,semid2;			/* return value from semget() */
struct  sembuf sop;
unsigned nsops;
message_buf sbuf;

// Function Declarations

void send_msg();
void file_read();
void file_write();
std::string get_working_path();

// Function Definitions

std::string get_working_path()
{
   char temp[200];
   return ( getcwd(temp, 200) ? std::string( temp ) : std::string("") );
}

// function for sending message to msg queue
void send_msg(int msqid)
{

  if (msgsnd(msqid, &sbuf, sizeof(struct msg), IPC_NOWAIT) < 0) 
  {
    perror("msgsnd");
    exit(1);
  }

  else 
  {
    printf("Message Sent to \"%ld\"  \n  ", sbuf.mtype);
    printf("successfully inserted in queue %d\n",msqid );
    if(msqid == msqid1)
    {
      s = get_working_path() + "/prod1.txt";
      f.open(s.c_str(), ios::in | ios::out);
      char * temp = new char[5];
      f>>temp;
      //printf("yo yo temp %s\n",temp);
      f.close();
      f.open(s.c_str(), ios::in | ios::out);
      int temp_num = atoi(temp);
      //printf("yo yo temp_num %d\n",temp_num);
      temp_num++;
      sprintf(temp,"%d",temp_num);
     // printf("yo yo temp_num %d\n",temp_num);
      f<<temp<<endl;
      f>>temp;
      //printf("yo yo temp %s\n",temp);
      delete []temp;
      f.close();
    }
    else
    {
      s = get_working_path() + "/prod2.txt";
      f.open(s.c_str(), ios::in | ios::out);
      char * temp = new char[5];
      f>>temp;
     // printf("yo yo temp %s\n",temp);
      f.close();
      f.open(s.c_str(), ios::in | ios::out);
      int temp_num = atoi(temp);
     // printf("yo yo temp_num %d\n",temp_num);
      temp_num++;
      sprintf(temp,"%d",temp_num);
     // printf("yo yo temp_num %d\n",temp_num);
      f<<temp<<endl;
      f>>temp;
      //printf("yo yo temp %s\n",temp);

      delete []temp;
      f.close();
    }
  }
}


void file_read()
{
  int i;
  s = get_working_path() + "/matrix.txt";
  f.open(s.c_str(), ios::in | ios::out );
  inbuf  = f.rdbuf();

  for(i=0;i<10;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q1[i] = atoi(&c);
  }	

  c = inbuf->sbumpc();
 // printf("%c",c);

  for(i=0;i<10;i++)
  {
    c = inbuf->sbumpc();
   // printf("%c",c);
    q2[i] = atoi(&c);
  }
 // printf("\n\n");
  f.close();
}

void file_write()
{   
  int i;
  char *tem;
  s = get_working_path() + "/matrix.txt";
  tem = (char*)malloc(sizeof(char)*10);
  f.open(s.c_str(), ios::in | ios::out );
  outbuf = f.rdbuf();  	

  for(i=0;i<10;i++)
  {
    sprintf(tem,"%d",q1[i]);
    outbuf->sputc (tem[0]);
  }	

  sprintf(tem,"\n");
  outbuf->sputc (tem[0]);

  for(i=0;i<10;i++)
  {
    sprintf(tem,"%d",q2[i]);
    outbuf->sputc (tem[0]);
  }

  f.close();	
}



int main(int argc,char *argv[])
{
  if(argc != 2)
  {
    printf("wrong number of arguments\n");
    exit(0);
  }	
  int n = 5;
  key1 = 1234;
  key2 = 1235;
  time_t t;
  int queue;
  int num;
  int id = atoi(argv[1]);
  printf("my id as producer %d\n",id);

  /* Intializes random number generator */
  srand((unsigned) time(&t));

  //joining msg queue
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

	key = (key_t)10;
	nsems = 1;

	semid = semget(key,nsems,IPC_CREAT|0666);
  if(semid == -1)
  {
  	 perror("semaphore creation failed");
  	 exit(1);
  }	
  q_key1 = (key_t)20;
  nsems = 1;

  semid1 = semget(q_key1,nsems,IPC_CREAT|0666);
  if(semid1 == -1)
  {
  	 perror("semaphore 1 creation failed");
  	 exit(1);
  }	

  q_key2 = (key_t)30;
  nsems = 1;

  semid2 = semget(q_key2,nsems,IPC_CREAT|0666);
  if(semid2 == -1)
  {
  	 perror("semaphore 2 creation failed");
  	 exit(1);
  }	
  
  
  while(1)
  {
    queue = rand() % 2;
    num = rand() % 50;
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;
    semop(semid, &sop, 1); // requesting access for file
    file_read(); 

    if(queue == 0)
    q1[id+5] = 1;
    else
    q2[id+5] = 1; 

    file_write();
    sop.sem_op = 1;

    semop(semid, &sop, 1); // releasing file
    printf("trying to insert in queue %d\n",(queue+1));

    sop.sem_op = -1;
    if(queue == 0)
    {
      semop(semid1,&sop,1);//requesting queue 1 access
      printf("inside if queue == 0\n access granted for q 1\n");
      sop.sem_op = -1;
      semop(semid, &sop, 1); // requesting access for file

      file_read(); 
      q1[id+5] = 2;
      file_write();
      sop.sem_op = 1;
      semop(semid, &sop, 1); // releasing file

      if (msgctl(msqid1, IPC_STAT, &buf) == -1) 
      {
        perror("msgctl: msgctl failed");
        exit(0);
      }
      printf("no of msg = %d in q1\n",(int)buf.msg_qnum);
      if(buf.msg_qnum < 10)
      {
       // printf("yay less than 10 msgs in q1\n");
        sbuf.mtype = 1;
        sbuf.messag.numb = num;
        sbuf.messag.pid = getpid(); 
        send_msg(msqid1);
      }

      sop.sem_op = 1;
      semop(semid1, &sop, 1); // releasing queue 1
      sop.sem_op = -1;
      semop(semid, &sop, 1); // requesting access for file

      file_read(); 
      q1[id+5] = 0;
      file_write();

      sop.sem_op = 1;
      semop(semid, &sop, 1); // releasing file
    } 
    else
    {
      semop(semid2,&sop,1); // requesting queue 2 access
      sop.sem_op = -1;
      semop(semid, &sop, 1); // requesting access for file
      file_read(); 

      q2[id+5] = 2;
      file_write();
      sop.sem_op = 1;
      semop(semid, &sop, 1); // releasing file
      if (msgctl(msqid2, IPC_STAT, &buf) == -1) 
      {
        perror("msgctl: msgctl failed");
        exit(1);
      }
      printf("no of msg = %d in q2\n",(int)buf.msg_qnum);
      if(buf.msg_qnum < 10)
      {
       // printf("yay less than 10 msgs in q2\n");
        sbuf.mtype = 1;
        sbuf.messag.numb = num;
        sbuf.messag.pid = getpid(); 
        send_msg(msqid2);
      }	

      sop.sem_op = 1;
      semop(semid2, &sop, 1); // releasing queue 2
      sop.sem_op = -1;
      semop(semid, &sop, 1); // requesting access for file

      file_read(); 
      q2[id+5] = 0;
      file_write();

      sop.sem_op = 1;
      semop(semid, &sop, 1); // releasing file
    }

    sleep(rand() % 3);
  }	
}