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
/*   sub semaphore mapping
      0 - North
      1 - West
      2 - South
      3 - East     */

#define MAX 30
#define PRO 50      
using namespace std;

// Global Variables
key_t key_me,key_d,key_f; 	/* key to pass to semget() */
int		nsems;	/* nsems to pass to semget() */
int semid_me,semid_d,semid_f;  	/* return value from semget() */
struct  sembuf sop;
int q0[MAX],q1[MAX],q2[MAX],q3[MAX]; //rows in matrix
string s;
fstream f;
filebuf* inbuf;
char c;
filebuf* outbuf;

// Function Declarations
std::string get_working_path();
void file_read();
void file_write();

// Function Definitions

void file_read()
{ 
  int i;
  f.open(s.c_str(), ios::in | ios::out );
  inbuf  = f.rdbuf();
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
   // printf("%c",c);
    q0[i] = atoi(&c);
  } 
  c = inbuf->sbumpc();
  //printf("%c",c);
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
   // printf("%c",c);
    q1[i] = atoi(&c);
  } 
  c = inbuf->sbumpc();
  //printf("%c",c);
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q2[i] = atoi(&c);
  } 
  c = inbuf->sbumpc();
  //printf("%c",c);
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q3[i] = atoi(&c);
  } 
  //printf("\n");
  f.close();
}

std::string get_working_path()
{
  char temp[200];
  return ( getcwd(temp, 200) ? std::string( temp ) : std::string("") );
}

void file_write()
{    
  int i;
  char *tem;
  tem = (char*)malloc(sizeof(char)*3);
  f.open(s.c_str(), ios::in | ios::out );
  outbuf = f.rdbuf();  	

  for(i=0;i<MAX;i++)
  {
    sprintf(tem,"%d",q0[i]);
    outbuf->sputc (tem[0]);
  }
  
  sprintf(tem,"\n");
  outbuf->sputc (tem[0]);
  
  for(i=0;i<MAX;i++)
  {
    sprintf(tem,"%d",q1[i]);
    outbuf->sputc (tem[0]);
  }
  
  sprintf(tem,"\n");
  outbuf->sputc (tem[0]);

  for(i=0;i<MAX;i++)
  {
    sprintf(tem,"%d",q2[i]);
    outbuf->sputc (tem[0]);
  }
  
  sprintf(tem,"\n");
  outbuf->sputc (tem[0]);

  for(i=0;i<MAX;i++)
  {
    sprintf(tem,"%d",q3[i]);
    outbuf->sputc (tem[0]);
  }
  
  sprintf(tem,"\n");
  outbuf->sputc (tem[0]);
  f.close();	
    
}

int main(int argc,char *argv[])
{
  if(argc < 3)
  {
  	printf("Arguments missing!\n");
  	exit(0);
  }
  char p_dir, dir = argv[1][0] ;
  //printf("%c \n", dir);
  int id = atoi(argv[2]),d,prio_d;
  if(dir == 'N')
  {  
    d = 0;
    p_dir = 'W';
   }
  else if(dir == 'W')
  {  
    d = 1;
    p_dir = 'S';
  }
  else if(dir == 'S')
  {  
    d = 2;
    p_dir = 'E';
  }
  else if(dir == 'E')
  {  
    d = 3;
    p_dir = 'N';
  }     
  key_f = (key_t)10;
  nsems = 1;
  semid_f = semget(key_f,nsems,IPC_CREAT|0666);
  if(semid_f == -1)
  {
    perror("semaphore creation failed");
    exit(1);
  }	
  
  key_d = (key_t)20;
  nsems = 4;
  ushort val[4]={1, 1, 1, 1};
  semid_d = semget(key_d,nsems,IPC_CREAT|0666);
  if(semid_d == -1)
  {
    perror("semaphore creation failed");
    exit(1);
  }	
  
  key_me = (key_t)30;
  nsems = 1;
  semid_me = semget(key_me,nsems,IPC_CREAT|0666);
  if(semid_me == -1)
  {
    perror("semaphore creation failed");
    exit(1);
  }	
  s = get_working_path() + "/matrix.txt";
  if(d == 3)
    prio_d = 0;
  else
    prio_d = (d+1);

  printf("%c : Train %d (PID %d) started\n",dir, id, (int)getpid());
  sop.sem_num = 0;  
  sop.sem_flg = 0;
  sop.sem_op = -1;
  semop(semid_f, &sop, 1); // requesting access for file
  file_read(); 
  
  if(dir == 'N')
    q0[id] = 1;
  else if(dir == 'W')
    q1[id] = 1;
  else if(dir == 'S')
    q2[id] = 1;
  else if(dir == 'E')
    q3[id] = 1;    
  
  file_write();

  sop.sem_op = 1;
  semop(semid_f, &sop, 1); // releasing file
  
  printf("%c : Train %d (PID %d) requesting access for own direction : %c\n",dir, id, (int)getpid(), dir);
  sop.sem_num = d;  
  sop.sem_op = -1;
  semop(semid_d,&sop,1);//requesting own direction semaphore access
  sop.sem_num = 0;  
  sop.sem_op = -1;
  printf("%c : Train %d (PID %d) access granted for own direction : %c\n",dir, id, (int)getpid(), dir);
  printf("%c : Train %d (PID %d) requesting access for prio_direction : %c.\n",dir, id, (int)getpid(), p_dir);
  semop(semid_f, &sop, 1); // requesting access for file
  file_read(); 
  if(dir == 'N')
  {  
    q0[id] = 2;
    q1[id] = 1;
   }
  else if(dir == 'W')
  {  
    q1[id] = 2;
    q2[id] = 1;
  }
  else if(dir == 'S')
  {  
    q2[id] = 2;
    q3[id] = 1;
  }
  else if(dir == 'E')
  {  
    q3[id] = 2;
    q0[id] = 1;
  }    
  file_write();
  sop.sem_op = 1;
  semop(semid_f, &sop, 1); // releasing file
  
  sop.sem_num = prio_d;  
  sop.sem_op = -1;
  semop(semid_d,&sop,1);//requesting prio direction semaphore access
  sop.sem_op = 1;
  printf("%c : Train %d (PID %d) access granted for prio_direction : %c.\n",dir, id, (int)getpid(), p_dir);
  semop(semid_d,&sop,1); // releasing prio direction after checking it
  sop.sem_num = 0;  
  sop.sem_op = -1;
  semop(semid_f, &sop, 1); // requesting access for file
  file_read(); 
  if(dir == 'N')
    q1[id] = 0;
  else if(dir == 'W')
    q2[id] = 0;
  else if(dir == 'S')
    q3[id] = 0;
  else if(dir == 'E')
    q0[id] = 0;    
  file_write();
  sop.sem_op = 1;
  semop(semid_f, &sop, 1); // releasing file
  
  printf("%c : Train %d (PID %d) requesting access for junction lock\n",dir, id, (int)getpid());
  sop.sem_num = 0;  
  sop.sem_op = -1;
  semop(semid_me, &sop, 1); // mutual exclusion access
  printf("%c : Train %d (PID %d) access granted for junction lock : passing junction\n",dir, id, (int)getpid());
  
  sleep(2);
  
  sop.sem_num = 0;  
  sop.sem_op = 1;
  semop(semid_me, &sop, 1); // mutual exclusion relieved
  printf("%c : Train %d (PID %d) released junction lock\n",dir, id, (int)getpid());

  sop.sem_num = d;  
  sop.sem_op = 1;
  semop(semid_d,&sop,1); // releasing own direction
  printf("%c : Train %d (PID %d) releases own direction : %c\n",dir, id, (int)getpid(), dir);
  sop.sem_num = 0;  
  sop.sem_op = -1;
  semop(semid_f, &sop, 1); // requesting access for file
  file_read(); 
  if(dir == 'N')
    q0[id] = 0;
  else if(dir == 'W')
    q1[id] = 0;
  else if(dir == 'S')
    q2[id] = 0;
  else if(dir == 'E')
    q3[id] = 0;    
  file_write();
  sop.sem_op = 1;
  semop(semid_f, &sop, 1); // releasing file
  return 0;
}
