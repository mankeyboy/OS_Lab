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

using namespace std;

#define IO_CALL SIGUSR1
#define TERM SIGUSR2
#define NOTIFY SIGINT
#define SUSPEND SIGTERM
//#define 4 4
#define RR_TQUANTA 1000
#define PR_TQUANTA 2000
#define MSGSZ 40




//Main message queue type
typedef struct msgbuff 
{
  long    mtype;
  struct msg
  {
    int prio;
    pid_t pid;
  }message;
}message_buf;

typedef struct process
{
  int pid;
  int prio;
  int sleep_prob, io_time;
  struct timeval start_time, temp_time, response_time;
  struct timeval twait_time, tprocessed_time, turnaround_time;
}process;



//Global variables
message_buf job_pool;
int msqid;
int start, end, w_start, w_end;
int msgflg = IPC_CREAT | 0666;//access flags
key_t key;
int n,count_proc;
int signaltermrecv = 0;
int signaliorecv = 0;
int sch_type;// 0: RR || 1 :PR
fstream f;
string s = "./result.txt", x; 
struct timeval avg_response , avg_wait , avg_turn ;



process ready_queue[4], running;
process wait_queue[4];
struct timeval temp;


static void signalreceived(int sig, siginfo_t *siginfo, void *context)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
   
  if(sig==IO_CALL)
  {
    if(siginfo->si_pid==running.pid)
    {
      cout<<"Process "<<running.pid<<" requests I/O\n";
      signaliorecv = 1;
    }
  }
  else if(sig==TERM)
  {
    if(siginfo->si_pid==running.pid)
    { 
      cout<<"Process "<<running.pid<<" terminated\n";  
      signaltermrecv = 1;
      count_proc++;
      gettimeofday(&temp,NULL);
      running.tprocessed_time.tv_sec += (temp.tv_sec - running.temp_time.tv_sec);
      running.tprocessed_time.tv_usec += (temp.tv_usec - running.temp_time.tv_usec);
       printf("at the end %d %d %d\n",count_proc,(int)running.start_time.tv_sec,(int)running.start_time.tv_sec);
      running.turnaround_time.tv_sec = (temp.tv_sec - running.start_time.tv_sec);
      running.turnaround_time.tv_usec = (temp.tv_usec - running.start_time.tv_usec);
      avg_response.tv_sec += (running.response_time.tv_sec);
      avg_response.tv_usec += (running.response_time.tv_usec);
      avg_wait.tv_sec += (running.twait_time.tv_sec);
      avg_wait.tv_usec += (running.twait_time.tv_usec);
      avg_turn.tv_sec += (running.turnaround_time.tv_sec);
      avg_turn.tv_usec += (running.turnaround_time.tv_usec);
      f.open(s.c_str(), ios::out | ios::app);
      float waiti_t,turn_t,res_t;
      res_t = (running.response_time.tv_sec + ((float)running.response_time.tv_usec/1000000));
      turn_t = (running.turnaround_time.tv_sec + ((float)running.turnaround_time.tv_usec/1000000 ));
      waiti_t = (running.twait_time.tv_sec + ((float)running.twait_time.tv_usec/1000000 ));

      f <<"PID: "<<running.pid<<"\n"<<"Response Time\n seconds "<<running.response_time.tv_sec<<"\nmicro seconds "
        <<running.response_time.tv_usec<<"\n"<<"Wait Time \n seconds "<<running.twait_time.tv_sec<<"\n"
        <<"micro seconds  "<<running.twait_time.tv_usec<<"\nTurnaround Time:  \nseconds"<<running.turnaround_time.tv_sec<<
        "\nmicro seconds "<<running.turnaround_time.tv_usec <<"\n response time  :"<<res_t<<"\n wait time  :" 
        <<waiti_t<<"\n turnaround time  : "<<turn_t<<endl;

      f.close();  
    }
    if(count_proc == 4)
    {
       float avg_waiting,avg_responsing,avg_turning;
       avg_waiting = ((avg_wait.tv_sec + ((float)avg_wait.tv_usec/1000000))/4);
       avg_responsing = ((avg_response.tv_sec + ((float)avg_response.tv_usec/1000000))/4);
       avg_turning = ((avg_turn.tv_sec + ((float)avg_turn.tv_usec/1000000))/4);
      //temp = clock();
      f.open(s.c_str(), ios::out | ios::app);
      f<<"Average Response: seconds "<<avg_response.tv_sec/4<<"\n"<<"micro seconds"<<avg_response.tv_usec/4
      <<"\n Average Wait seconds: "<<avg_wait.tv_sec/4<<"\n"<<"micro seconds"<<avg_wait.tv_usec/4
      <<"\n Average Turnaround seconds: "<<avg_turn.tv_sec/4<<"\nmicroseconds "<<avg_turn.tv_usec/4<<"\n Average Response  :"
      <<avg_responsing<<"\n  Average Wait : "<<avg_waiting<<"\n  Average Turnaround : "<<avg_turning<<endl;
      f.close();
      msgctl(msqid, IPC_RMID, NULL);
      exit(0);
    }   
  }  

}


void allocate()
{
  if(ready_queue[end].prio == 10)
  {
    ready_queue[end].sleep_prob = 30;
    ready_queue[end].io_time = 1;
  }
  else if(ready_queue[end].prio == 5)
  {
    ready_queue[end].sleep_prob = 70;
    ready_queue[end].io_time = 3;
  }
}

int exists(pid_t temp_pid)
{
  for(int i = 0; i<4;  i++)
  {
    if(temp_pid == wait_queue[i].pid)
      return i;
    else return -1;
  }
}

void initialize()
{
  for(int i = 0; i<4; i++)
  {
    ready_queue[i].twait_time.tv_sec = wait_queue[i].twait_time.tv_sec = 0;
    ready_queue[i].tprocessed_time.tv_sec = wait_queue[i].tprocessed_time.tv_sec = 0;
    ready_queue[i].turnaround_time.tv_sec = wait_queue[i].turnaround_time.tv_sec = 0;
    ready_queue[i].start_time.tv_sec = wait_queue[i].start_time.tv_sec = 0;
    ready_queue[i].temp_time.tv_sec = wait_queue[i].temp_time.tv_sec = 0;
    ready_queue[i].response_time.tv_sec = wait_queue[i].response_time.tv_sec = 0;

    ready_queue[i].twait_time.tv_usec = wait_queue[i].twait_time.tv_usec = 0;
    ready_queue[i].tprocessed_time.tv_usec = wait_queue[i].tprocessed_time.tv_usec = 0;
    ready_queue[i].turnaround_time.tv_usec = wait_queue[i].turnaround_time.tv_usec = 0;
    ready_queue[i].start_time.tv_usec = wait_queue[i].start_time.tv_usec = 0;
    ready_queue[i].temp_time.tv_usec = wait_queue[i].temp_time.tv_usec = 0;
    ready_queue[i].response_time.tv_usec = wait_queue[i].response_time.tv_usec = 0;
    
    ready_queue[i].pid = wait_queue[i].pid = -1;
    ready_queue[i].prio = wait_queue[i].prio = 20;
  }
}

void sort()
{
  process tempp;
  int c;
  int i,j,t;
  c = 0;
  for( i= 0; i< 4; i++)
  {
    
    
        for( j = (i+1); j<4; j++)
      {
        
        if(ready_queue[i].prio > ready_queue[j].prio)
        {
          
          tempp = ready_queue[i];
          ready_queue[i] = ready_queue[j];
          ready_queue[j] = tempp;
        }  
      }
    
  }
  for( t = 0;t<4;t++)
  {
    if(ready_queue[t].prio < 20) 
      c = (c+1);
  } 
  start = 0;
  end = c;
  printf("start = %d end = %d   %d\n ",start,end,c);
}
//scheduling function
void schedule()
{
  avg_response.tv_sec = 0;
  avg_response.tv_usec = 0;
  avg_wait.tv_sec = 0;
  avg_wait.tv_usec = 0;
  avg_turn.tv_sec = 0;
  avg_turn.tv_usec = 0;
  
  gettimeofday(&temp,NULL);

  //gettimeofday(&(ready_queue[end].start_time),NULL);
  //ready_queue[end].start_time = clock();
  ready_queue[end].start_time.tv_sec = temp.tv_sec;
  ready_queue[end].start_time.tv_usec = temp.tv_usec;
  printf("at the start %d %d %d\n",end,(int)ready_queue[end].start_time.tv_sec,(int)ready_queue[end].start_time.tv_sec);
  ready_queue[end].turnaround_time.tv_sec = 0;
  ready_queue[end].turnaround_time.tv_usec = 0;
  

  ready_queue[end].temp_time.tv_sec = temp.tv_sec;
  ready_queue[end].temp_time.tv_usec = temp.tv_usec;
  ready_queue[end].pid = job_pool.message.pid;
  ready_queue[end].prio = job_pool.message.prio;
  allocate();
  end++;
  int flag = 0;
  message_buf receivedjob;
  
     
  while(1)
  {
    
    
    if(msgrcv(msqid,&receivedjob,sizeof(message_buf),1,IPC_NOWAIT)!=-1)
    {
      process p;
      p.pid = receivedjob.message.pid;
      p.prio = receivedjob.message.prio;
      printf("recieved process %d\n", (int)p.pid);
      //For back from IO
      int place;
      if ((place = exists(p.pid))!= -1)
      {
        ready_queue[end] = wait_queue[place];
        gettimeofday(&temp,NULL);
        ready_queue[end].temp_time.tv_sec = temp.tv_sec;
        ready_queue[end].temp_time.tv_usec = temp.tv_usec;  
        if(end == 4 )
          end = 0;
        else end++;
        printf("%d place\n",place);
        wait_queue[place].pid = -1;
        printf("in place \n");
      }
      //For New process
      else
      {
        gettimeofday(&temp,NULL);
        //gettimeofday(&(ready_queue[end].start_time),NULL);
        //ready_queue[end].start_time = clock();
        ready_queue[end].start_time.tv_sec = temp.tv_sec;
        ready_queue[end].start_time.tv_usec = temp.tv_usec;
        printf("at the start %d %d %d\n",end,(int)ready_queue[end].start_time.tv_sec,(int)ready_queue[end].start_time.tv_sec);
        ready_queue[end].turnaround_time.tv_sec = 0;
        ready_queue[end].turnaround_time.tv_usec = 0;
  
        ready_queue[end].temp_time.tv_sec = temp.tv_sec;
        ready_queue[end].temp_time.tv_usec = temp.tv_usec;
        ready_queue[end].pid = p.pid;
        ready_queue[end].prio = p.prio;
        allocate();
        printf("new process allocated place in ready_queue pid = %d\n",(int)ready_queue[end].pid);
        if(end == 4)
          end = 0;
        else end++;
      }
    }
    
    if(sch_type == 0)
    {
      
      if(start == end)
        continue;
      gettimeofday(&temp,NULL); 
      //temp = clock();
      ready_queue[start].twait_time.tv_sec += (temp.tv_sec - ready_queue[start].temp_time.tv_sec);
      ready_queue[start].temp_time.tv_sec = temp.tv_sec;
      ready_queue[start].twait_time.tv_usec += (temp.tv_usec - ready_queue[start].temp_time.tv_usec);
      ready_queue[start].temp_time.tv_usec = temp.tv_usec;

      if(ready_queue[start].tprocessed_time.tv_sec == 0 && ready_queue[start].tprocessed_time.tv_usec == 0)
      {
        ready_queue[start].response_time.tv_sec = (temp.tv_sec - ready_queue[start].start_time.tv_sec);
        ready_queue[start].response_time.tv_usec = (temp.tv_usec - ready_queue[start].start_time.tv_usec);
      }
      running = ready_queue[start];
      ready_queue[start].prio = 20;
      if(start == 4)
        start = 0;
      else
        start++;

      if(kill(running.pid,NOTIFY)==-1)
        perror("Sending Notify ");
      else
        printf("Process %d is running\n", running.pid);

      for(int j = 0; j<RR_TQUANTA; j++)
      {
        if((signaltermrecv != 0) || (signaliorecv != 0) )
          break;
        usleep(500);
      }
      

      if(signaltermrecv)
      {
        flag = 1;
        /*temp = clock();
        running.tprocessed_time += temp - running.temp_time;
        running.turnaround_time = temp - running.start_time;
        avg_response += running.response_time;
        avg_wait += running.twait_time;
        avg_turn += running.turnaround_time;
        
        

        f.open(s.c_str(),ios::out | ios::app );

        f <<"PID: "<<running.pid<<"\n"<<"Response Time"<<running.response_time<<"\n"
          <<"Wait Time "<<running.twait_time<<"\n"<<"Turnaround Time: "
          <<running.turnaround_time<<"\n"<<endl; */

       // if(count_proc == 4)
         // f<<"Average Response: "<<avg_response/4<<"\n"<<"Average Wait: "
          //<<avg_wait/4<<"\n"<<"Average Turnaround: "<<avg_turn/4;
        //f.close();
        signaltermrecv = 0;
        //if(count_proc == 4)
          //exit(0);
      }
      if(signaliorecv)
      {
        flag = 1;
        gettimeofday(&temp,NULL); 
        //temp = clock();
        running.tprocessed_time.tv_sec += (temp.tv_sec - running.temp_time.tv_sec);
        running.tprocessed_time.tv_usec += (temp.tv_sec - running.temp_time.tv_usec);
        for(int k = 0; k< 4; k++)
        {
          if(wait_queue[k].pid == -1)
          {
            wait_queue[k] = running;
            printf("pid changed %d\n",(int)wait_queue[k].pid);
            break;
          }
        }       
        signaliorecv = 0;
      }
      if(flag == 1)
      {
        flag = 0;
      } 
      else
      {
        if(kill(running.pid,SUSPEND)==-1)
            perror("Sending Suspend ");
        else
            printf("Process %d is suspended\n", running.pid);
      } 
    }

    if(sch_type == 1)
    {

      sort();
       if(start == end)
        {printf("executing continue\n");
         continue;}
      gettimeofday(&temp,NULL);
      //temp = clock();
      ready_queue[start].twait_time.tv_sec += (temp.tv_sec - ready_queue[start].temp_time.tv_sec);
      ready_queue[start].twait_time.tv_usec += (temp.tv_usec - ready_queue[start].temp_time.tv_usec);
      
      ready_queue[start].temp_time.tv_sec = temp.tv_sec;
      ready_queue[start].temp_time.tv_usec = temp.tv_usec;

      if(ready_queue[start].tprocessed_time.tv_sec == 0 && ready_queue[start].tprocessed_time.tv_usec == 0 )
      {
        ready_queue[start].response_time.tv_sec = temp.tv_sec - ready_queue[start].start_time.tv_sec;
        ready_queue[start].response_time.tv_usec = temp.tv_usec - ready_queue[start].start_time.tv_usec;
      }  
      running = ready_queue[start];
      ready_queue[start].prio = 20;
      if(start == 3)
        start = 0;
      else
        start++;

      if(kill(running.pid,NOTIFY)==-1)
        perror("Sending Notify ");
      else
        printf("Process %d is running\n", running.pid);

      for(int j = 0;j<PR_TQUANTA; j++)
      {
        if((signaltermrecv != 0) || (signaliorecv != 0) )
          break;
        usleep(50);
      }

      if(signaltermrecv)
      { 
        flag = 1;
        /*temp = clock();
        running.tprocessed_time += temp - running.temp_time;
        running.turnaround_time = temp - running.start_time;
        avg_response += running.response_time;
        avg_wait += running.twait_time;
        avg_turn += running.turnaround_time;
        
       
        f.open(s.c_str(), ios::out | ios::app);

        f <<"PID: "<<running.pid<<"\n"<<"Response Time"<<running.response_time<<"\n"
          <<"Wait Time "<<running.twait_time<<"\n"<<"Turnaround Time: "
          <<running.turnaround_time<<"\n"<<endl; */

       // if(count_proc == 4)
         // f<<"Average Response: "<<avg_response/4<<"\n"<<"Average Wait: "
          //<<avg_wait/4<<"\n"<<"Average Turnaround: "<<avg_turn/4;
       // f.close();
        signaltermrecv = 0;
        //if(count_proc == 4)
          //exit(0);
      }
      if(signaliorecv)
      { 
        flag = 1;
        gettimeofday(&temp,NULL); 
        //temp = clock();
        running.tprocessed_time.tv_sec += (temp.tv_sec - running.temp_time.tv_sec);
        running.tprocessed_time.tv_usec += (temp.tv_usec - running.temp_time.tv_usec);
        for(int k = 0; k< 4; k++)
        {
          if(wait_queue[k].pid == -1)
          {
            wait_queue[k] = running;
            printf("pid changed %d\n",(int)wait_queue[k].pid);

            break;
          }
        }       
        signaliorecv = 0;
      }
      if(flag == 1)
      {
        flag = 0;
      } 
      else
      {
        if(kill(running.pid,SUSPEND)==-1)
            perror("Sending Suspend ");
        else
            printf("Process %d is suspended\n", running.pid);
      } 
    }


  }

}

int main(int argc,char * argv[])
{
  
  if(argc != 3)
  { 
    perror("Wrong number of arguments");
    exit(-1);
  }

  struct sigaction sa;
  sa.sa_sigaction=&signalreceived;
  //sa.sa_handler = sigint_handler;
  //sa.sa_flags = SA_RESTART; // or SA_RESTART
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);

  if ( sigaction(IO_CALL, &sa, NULL) == -1 || sigaction(TERM,&sa,NULL)==-1) 
  {
    perror("sigaction");
    exit(1);
  }

  key = 1234;
  int stat;
  count_proc = 0;
  
  n = atoi(argv[1]);
  sch_type = atoi(argv[2]);

  start = end = w_start = w_end =  0;

  initialize();
  //creating msg queue
  if ((msqid = msgget(key, msgflg )) < 0) 
  {
    perror("msgget");
    exit(1);      
  }
  
  else 
    printf("message queue created : %d\n",msqid);
   
 
  stat =  msgrcv(msqid, &job_pool, sizeof(message_buf), 1,MSG_NOERROR);
  if(stat < 0)
  {
    perror("msgrcv");
    exit(1);
  }
  else
    schedule();
  
  return 0;

}