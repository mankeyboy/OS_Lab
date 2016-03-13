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

#define PRO 80

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


//Global variables
message_buf mes;
int msqid1,msqid2;
int msgflg = IPC_CREAT | 0666;//access flags
key_t key1,key2;
struct msqid_ds buf;
int q1[10],q2[10];
string s ;
fstream f;
filebuf* inbuf;
filebuf* outbuf;
char c;
key_t   key,q_key1,q_key2;			/* key to pass to semget() */
int		nsems;			                /* nsems to pass to semget() */
int		semid,semid1,semid2;			/* return value from semget() */
struct  sembuf sop;
unsigned nsops;
int prob;
int c1=0,c2=0;

// Function Declarations
void file_read();
void file_write();
void msg_rcv();
std::string get_working_path();

//Function Definitions
std::string get_working_path()
{
   char temp[200];
   return ( getcwd(temp, 200) ? std::string( temp ) : std::string("") );
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
    //printf("%c",c);
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
  tem = (char*)malloc(sizeof(char)*5);
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

void msg_rcv(int msqid)
{
   int counter,qu;
   if(msgrcv(msqid,&mes,sizeof(message_buf),1,IPC_NOWAIT)!=-1)
   {
    printf("number removed from %d \n",msqid);
    if(msqid == msqid1)
    {
      c1 = counter = (c1+1);
      qu = 1;
    } 
    else
    {
      c2 = counter = (c2+1);
      qu = 2;
    } 
    printf("queue - %d | counter - %d | number - %d\n",qu,counter,mes.message.numb);
    if(msqid == msqid1)
    {
      s = get_working_path() + "/cons1.txt";
      f.open(s.c_str(), ios::in | ios::out);
      char * temp = new char[5];
      f>>temp;
      //printf("yo yo temp %s\n",temp);
      f.close();
      f.open(s.c_str(), ios::in | ios::out);
      int temp_num = atoi(temp);
      //printf("yo yo temp_num %d\n",temp_num);
      temp_num++;
      //printf("yo yo temp_num %d\n",temp_num);
      f<<temp_num<<endl;
      delete []temp;
      f.close();
    }
    else
    {
      s = get_working_path() + "/cons2.txt";
      f.open(s.c_str(), ios::in | ios::out);
      char * temp = new char[5];
      f>>temp;
      //printf("yo yo temp %s\n",temp);
      f.close();
      f.open(s.c_str(), ios::in | ios::out);
      int temp_num = atoi(temp);
      //printf("yo yo temp_num %d\n",temp_num);
      temp_num++;
      //printf("yo yo temp_num %d\n",temp_num);
      f<<temp_num<<endl;
      delete []temp;
      f.close();
    }
   }
  else
  {
    perror("error in recieving");
    exit(1);
  }	
}

// MAIN FUNCTION

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
  printf("my id %d\n",id);

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
  
  // MAIN LOOP
  while(1)
  {
    sleep(rand()%3);
    prob = rand()% 100;
    queue = rand()% 2;
    sop.sem_num = 0;	
    sop.sem_flg = 0;
    if(prob < PRO)
    {
      sop.sem_op = -1;
      semop(semid, &sop, 1); // requesting access for file
      file_read(); 
      if(queue == 0)
      q1[id] = 1;
      else
      q2[id] = 1; 

      file_write();

      sop.sem_op = 1;
      semop(semid, &sop, 1); // releasing file
      sop.sem_op = -1;
      if(queue == 0)
      {
        semop(semid1,&sop,1);//requesting queue 1 access
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q1[id] = 2;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
        
        if (msgctl(msqid1, IPC_STAT, &buf) == -1) 
        {
          perror("msgctl: msgctl failed");
          exit(0);
        }

        if(buf.msg_qnum > 0)
        {  
          msg_rcv(msqid1);
        }
        else 
        sleep(1); 
        
        sop.sem_op = 1;
        semop(semid1, &sop, 1); // releasing queue 1
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q1[id] = 0;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
      }
      else
      {
        semop(semid2,&sop,1);//requesting queue 2 access
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q2[id] = 2;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
         
        if (msgctl(msqid2, IPC_STAT, &buf) == -1) 
        {
          perror("msgctl: msgctl failed");
          exit(1);
        }

        if(buf.msg_qnum > 0)
        {  
          msg_rcv(msqid2);
        }
        else
        sleep(1);  
        
        sop.sem_op = 1;
        semop(semid2, &sop, 1); // releasing queue 2
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read();
        q2[id] = 0;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
     }
    }
    else
    {
      if(queue == 0)
      {
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q1[id] = 1;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file 

        sop.sem_op = -1;
        semop(semid1,&sop,1);//requesting queue 1 access

        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file

        file_read(); 
        q1[id] = 2;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
        
         
        if (msgctl(msqid1, IPC_STAT, &buf) == -1) 
        {
          perror("msgctl: msgctl failed");
          exit(1);
        }

        if(buf.msg_qnum > 0)
        {  
          msg_rcv(msqid1);
        }
        else
        sleep(1);
        


        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q2[id] = 1;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file 
        sop.sem_op = -1;
        semop(semid2,&sop,1);//requesting queue 2 access
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q2[id] = 2;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
        
       
        if (msgctl(msqid2, IPC_STAT, &buf) == -1) 
        {
          perror("msgctl: msgctl failed");
          exit(1);
        }

        if(buf.msg_qnum > 0)
        {  
          msg_rcv(msqid2);
        }
        else
        sleep(1);
      
        sop.sem_op = 1;
        semop(semid2, &sop, 1); // releasing queue 2
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q2[id] = 0;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file

        sop.sem_op = 1;
        semop(semid1, &sop, 1); // releasing queue 1

        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q1[id] = 0;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
      }
      else
      {
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q2[id] = 1;
        file_write();

        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file 

        sop.sem_op = -1;
        semop(semid2,&sop,1);//requesting queue 2 access

        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q2[id] = 2;
        file_write();

        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
        
        if (msgctl(msqid2, IPC_STAT, &buf) == -1) 
        {
          perror("msgctl: msgctl failed");
          exit(1);
        }

        if(buf.msg_qnum > 0)
        {  
          msg_rcv(msqid2);
        }
        else
        sleep(1);
        
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q1[id] = 1;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file 
        sop.sem_op = -1;
        semop(semid1,&sop,1);//requesting queue 1 access
        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 
        q1[id] = 2;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
      
        if (msgctl(msqid1, IPC_STAT, &buf) == -1) 
        {
          perror("msgctl: msgctl failed");
          exit(1);
        }

        if(buf.msg_qnum > 0)
        {  
          msg_rcv(msqid1);
        }
        else
        sleep(1);
          
        sop.sem_op = 1;
        semop(semid1, &sop, 1); // releasing queue 1

        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file
        file_read(); 

        q1[id] = 0;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
 
        sop.sem_op = 1;
        semop(semid2, &sop, 1); // releasing queue 2

        sop.sem_op = -1;
        semop(semid, &sop, 1); // requesting access for file

        file_read(); 
        q2[id] = 0;
        file_write();
        sop.sem_op = 1;
        semop(semid, &sop, 1); // releasing file
      
      }	
    }	
  }
  return 0;	
}