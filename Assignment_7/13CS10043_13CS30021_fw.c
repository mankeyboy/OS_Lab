/*
13CS10043 S Raagapriya
13CS30021 Mayank Roy
*/

#include <stdio.h>
#include <pthread.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>





#define INTVAL 1000000000

//Global Variables

int n,m;
int dist[100][100];
int read_count = 0;
key_t key;
int		semid;			/* return value from semget() */

typedef struct arg
{
	int i,j,k;
}argument;

// Functions

void sigint_handler(int sig)
{
  semctl(semid, 0, IPC_RMID, 0);
  exit(1);
} 

void intialize()
{
  int a,b;
  for(a =0;a<100;a++)
  {
     for(b=0;b<100;b++)
     {
     	 if(a == b)
     	 	 dist[a][b] = 0;
     	 else
     	   dist[a][b] = INTVAL;	
     }	
  }	
}

void down(int subsem)
{
	struct sembuf sop;
	sop.sem_num = subsem;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	if(semop(semid,&sop,1)<0)
		perror("Semop error ");
}
void up(int subsem)
{
	struct sembuf sop;
	sop.sem_num = subsem;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	if(semop(semid,&sop,1)<0)
		perror("Semop error ");
}

int read_val(int i, int j)
{
	int val;

	down(0);
	read_count++;
	if(read_count==1)
		down(1);
	up(0);

	/* Reading is performed in the variable val*/
	val = dist[i][j];

	down(0);
	read_count--;
	if(read_count==0)
		up(1);
	up(0);

	return val;
}

void write_val(int k, int j, int val)
{
	down(1);

	/* Writing is performed */
	dist[k][j] = val;
	dist[j][k] = val;

	up(1);
}


void printMatrix()
{
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			if(dist[i][j]==INTVAL)
				printf("INF ");
			else
				printf("%d   ",dist[i][j]);
		}
		printf("\n");
	}
}

void* start_routine(void *args)
{
	int i,j,k,min,ji,ik,jk;
	argument temp = *(argument *)args;
	i = temp.i;
	j = temp.j;
	for(k=0;k<n;k++)
	{  
    ji = read_val(j,i);
    ik = read_val(i,k);
    jk = read_val(j,k);
    if((ji + ik) < jk)
    {
     	write_val(k,j,(ji + ik));
    }	

	} 
}

int main(int argc,char * argv[])
{
	int		nsems;			/* nsems to pass to semget() */
	
	struct  sembuf sop;
	unsigned nsops;
  if(argc != 3)
  {
   	printf("invalid number of arguments\n");
    exit(0);
  }

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


  key = (key_t)20;
  nsems = 2;
  semid = semget(key,nsems,IPC_CREAT|0666);
  if(semid == -1)
  {
    perror("semaphore 1 creation failed");
    exit(1);
  } 
  semctl(semid, 0, SETVAL, 1);
  semctl(semid, 1, SETVAL, 1);

  int i,k,j;

  n = atoi(argv[1]);
  m = atoi(argv[2]);
  
  intialize();

  int e1,e2,wt;
  
  for(i=0;i<m;i++)
  {
    scanf("%d %d %d",&e1,&e2,&wt);
    dist[e1-1][e2-1] = wt;
    dist[e2-1][e1-1] = wt; 
  }
  
  pthread_t threads[n];
  argument ar[n];

  for(i=0;i<n;i++)
  {
  	for(j=0;j<n;j++)
  	{
  		ar[j].i=i;
			ar[j].j=j;
			ar[j].k = -1;
			if(pthread_create(&threads[j],NULL,start_routine,(void *)&ar[j])!=0)
				perror("Creating thread ");
  	}
  	for(j=0;j<n;j++)
  	{
  		if(pthread_join(threads[j],NULL)!=0)
				perror("joining thread ");	
  	}
  }	
  printMatrix();
}