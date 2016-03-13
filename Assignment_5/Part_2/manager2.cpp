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
int num_proc;
int q0[MAX],q1[MAX],q2[MAX],q3[MAX]; //rows in matrix
string s;
fstream f;
filebuf* inbuf;
char c;
string seq;
fstream ip;
filebuf *inbuf_ip;
char seque[MAX];  // input sequence of trains
int pid_arr[MAX];
int temp_count = 0;
int pn,ps,pw,pe;
std::string get_working_path();
// Function Definitions

void sigint_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
  semctl(semid_me, 0, IPC_RMID, 0);
  semctl(semid_f, 0, IPC_RMID, 0);
  semctl(semid_d, 0, IPC_RMID, 0);
  exit(1);
}

void initialize()
{
	int i;
	for(i=0;i<MAX;i++)
	{
		q0[i] = q1[i] = q2[i] = q3[i] = 0;
		seque[i] = '\0';
	}

}


void input_file_read()
{ 
  int i;
  s = get_working_path() + "/sequence.txt";
  f.open(s.c_str(), ios::in | ios::out );
  inbuf  = f.rdbuf();
  num_proc = 0;
  c = inbuf->sbumpc();
  while(c != EOF)
  {
     seque[num_proc] = c;
     printf("c - %c seque[%d] - %c\n",c,num_proc,seque[num_proc]);
     num_proc++;
     c = inbuf->sbumpc();
  }
  printf("\n");
  f.close();
}

void file_read()
{ 
  int i;
  s = get_working_path() + "/matrix.txt";
  f.open(s.c_str(), ios::in | ios::out );
  inbuf  = f.rdbuf();
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
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
 // printf("%c",c);
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q2[i] = atoi(&c);
  } 
  c = inbuf->sbumpc();
 // printf("%c",c);
  for(i=0;i<MAX;i++)
  {
    c = inbuf->sbumpc();
    //printf("%c",c);
    q3[i] = atoi(&c);
  } 
 // printf("\n");
  f.close();
}

std::string get_working_path()
{
  char temp[200];
  return ( getcwd(temp, 200) ? std::string( temp ) : std::string("") );
}

void check_deadlock()
{
    int i;
    pn = ps = pw = pe = -1;
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;
    semop(semid_f, &sop, 1); // requesting access for file
    
    file_read();
    for(i=0;i<MAX;i++)
    {
       if((q0[i] == 2) && (q1[i] == 1))
         pn = i;
       if((q1[i] == 2) && (q2[i] == 1))
         pw = i;
       if((q2[i] == 2) && (q3[i] == 1))
         ps = i;
       if((q3[i] == 2) && (q0[i] == 1))
         pe = i;
    }
    if((pn != -1) && (pw != -1) && (ps != -1) && (pe != -1))
    {
      printf("Deadlock detected\n");
      printf("N --> NT(PID %d) --> W --> WT(PID %d) --> S --> ST(PID %d) --> E --> ET(PID %d) --> N\n",pid_arr[pn],pid_arr[pw],pid_arr[ps],pid_arr[pe]);
      semctl(semid_me, 0, IPC_RMID, 0);
      semctl(semid_f, 0, IPC_RMID, 0);
      semctl(semid_d, 0, IPC_RMID, 0);
      killpg(0, SIGKILL);
    }
    sop.sem_op = 1;
    semop(semid_f, &sop, 1); // releasing file

}

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
  time_t t;
  /* Intializes random number generator */
  srand((unsigned) time(&t));
  key_f = (key_t)10;
  nsems = 1;
  semid_f = semget(key_f,nsems,IPC_CREAT|0666);
  if(semid_f == -1)
  {
    perror("semaphore 1 creation failed");
    exit(1);
  }	
  semctl(semid_f, 0, SETVAL, 1);
  
  key_d = (key_t)20;
  nsems = 4;
  ushort val[4]={1, 1, 1, 1};
  semid_d = semget(key_d,nsems,IPC_CREAT|0666);
  if(semid_d == -1)
  {
    perror("semaphore 2 creation failed");
    exit(1);
  }	
  semctl(semid_d, 0, SETALL, val);

  key_me = (key_t)30;
  nsems = 1;
  semid_me = semget(key_me,nsems,IPC_CREAT|0666);
  if(semid_me == -1)
  {
    perror("semaphore 3 creation failed");
    exit(1);
  }	
  semctl(semid_me, 0, SETVAL, 1);

  s = get_working_path() + "/matrix.txt";
  f.open(s.c_str(), ios::in | ios::out | ios::trunc);
  f<<"000000000000000000000000000000"<<"\n"
   <<"000000000000000000000000000000"<<"\n"
   <<"000000000000000000000000000000"<<"\n"
   <<"000000000000000000000000000000"<<"\n"<<endl;
  f.close();
  initialize();
  input_file_read();
  int ran,ret,i;
  pid_t pid;
  char *args[4];
  for(i=0;i<4;i++)
  {
    args[i] = (char*)malloc(50*sizeof(char));
  }  
  while(1)
  {
       ran = rand()%100;
       if((ran < PRO) || (temp_count >= num_proc))
       {
       	  check_deadlock();
       	  if(temp_count >= num_proc)
       	    sleep(1);	
       }
       else
       {
       	   pid = fork();
           if(pid == 0)
           {
              strcpy(args[0],"./train.out");
              printf("%s || ", args[0]);
              sprintf(args[1],"%c", seque[temp_count]);
              printf("%s || ", args[1]);
              sprintf(args[2],"%d", temp_count);
              printf("%s |\n", args[2]);
              args[3] = NULL;
              ret = execvp(args[0],args);
              if(ret < 0)
              {
                printf("*** ERROR: exec failed\n");
                exit(1);
              } 
              break; 
           }
           else
           {
             pid_arr[temp_count] = (int)pid;
             temp_count++;
             usleep(100);
           } 
       }	
  }	




}