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

// Macro Declarations
#define SHMSZ 1024
#define TRANSSZ 50
#define ATMMAX 50

// Structures Used
typedef struct msg
{
    long balance;
    pid_t client_pid;
    int transaction_amnt;
    int req_type;
    int ret_type;
}request_data;

typedef struct msgbuff 
{
    long mtype;
    request_data request;
}message_buf;

typedef struct msg request_data;

typedef struct transact_data
{
    time_t t_trans;
    int client_pid;
    int amnt;
    int dep_or_withdraw;
}transaction;

typedef struct client_info
{
    pid_t pid;
    vector <transaction> trans_history;
    long balance;
    int is_checked;
}client;

typedef struct shared_mem_data
{
    time_t time_stamp;
    pid_t current_pid;
    long balance;
    int num;
    transaction current_session[TRANSSZ];
}shared_mem;

//Global Variables

int ATM_num;                              // Number of ATMs
key_t ATM_keys[ATMMAX][4];       // KeyDump;  See initialize()
vector <client> clients;                // Client data
string ATM_Locator;                     // ATM Locator File path
int msqid[ATMMAX];
int shmid[ATMMAX];
message_buf ATM_buf;
shared_mem *data;
struct msqid_ds qinfo;
//int flag;

// Function Declarations

void initialize();
string get_working_path();
void put_ATMfile_data();
void msg_qcreate();
void generate();
void global_consistency(int id);
int check_and_add();
void shm_create();

// Function Definitions

void shm_create()
{
    for(int i = 0; i< ATM_num; i++)
    {
        if ((shmid[i] = shmget(ATM_keys[i][2], SHMSZ, IPC_CREAT | 0666)) < 0) 
        {
            perror("shmget");
            exit(1);
        }
         // else 
         //    printf("Shared memory %d created : %d\n",i, shmid[i]);
    }      
}

void sigint_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
    for(int i = 0; i < ATM_num; i++)
    {
        msgctl(msqid[i], IPC_RMID, NULL);
        //shmctl(shmid[i], IPC_RMID, NULL);
    }
    //killpg(0,SIGKILL);
    exit(1);
}

int check_and_add()
{
    int i;
    for(i = 0; i< clients.size(); i++)
    {   
        if(ATM_buf.request.client_pid == clients[i].pid)
        {
            ATM_buf.request.balance = clients[i].balance;
            return 100;                  // 100 Status for successful joining
        }
    }
    if(i == clients.size())
    {
        client temp;
        temp.pid = ATM_buf.request.client_pid;
        temp.balance = 0;
        temp.is_checked = 1;
        clients.push_back(temp);
        return 101;
    }
}

void generate()
{
    msg_qcreate();
    
    int pid;
    char * command = new char [50];
    for(int i=0; i<ATM_num ;i++)
    {
        pid = (int)fork();

        if(pid == 0)
        {
            printf("Executing ATM %d \n",i+1);
            sprintf(command,"./atm %d %d %d %d %d %d",i+1, (int)ATM_keys[i][0], (int)ATM_keys[i][1], (int)ATM_keys[i][2], (int)ATM_keys[i][3], ATM_num);
            system(command);
            printf("After sys call\nATM %d closed! \n",i+1);
            exit(0);
        }
        else
        {   

            sleep(1);
        }  
    }
    put_ATMfile_data();
    while(1)
    {
        int flag = 0;
        for(int i = 0; i< ATM_num && flag ==0; i++)
            if(msgrcv(msqid[i], &ATM_buf,sizeof(struct msg),1,IPC_NOWAIT)!=-1)
                flag = i+1;
         
        if(flag != 0)
        {
            //cout<<"Request received by Master!\n";
            cout<<ATM_buf.request.req_type<<" : "<<ATM_buf.request.client_pid<<endl;
            if(ATM_buf.request.req_type == 1)// Joining
            {
                ATM_buf.request.ret_type = check_and_add();
                //cout<<"Sending Ret type:"<<ATM_buf.request.ret_type<<" to ATM for client"<<ATM_buf.request.client_pid<<endl;    
                if (msgsnd(msqid[flag -1], &ATM_buf, sizeof(struct msg), 0) < 0) 
                {
                    perror("msgsnd");
                    exit(1);
                }
                else 
                {
                    //printf("Message Sent to \"%ld\"  \n  ", ATM_buf.mtype);
                    //printf("Successfully inserted in queue %d\n",msqid[flag - 1] );
                }                                      
            }
            else if(ATM_buf.request.req_type == 4)// Global Consistency Check
            {
                global_consistency(flag -1);
                ATM_buf.request.ret_type = 400;
                if (msgsnd(msqid[flag -1], &ATM_buf, sizeof(struct msg), 0) < 0) 
                {
                    perror("msgsnd");
                    exit(1);
                }
                else 
                {
                    //printf("Message Sent to \"%ld\"  \n  ", ATM_buf.mtype);
                    //printf("Successfully inserted in queue %d\n",msqid[flag - 1] );
                }
            }
        }    
    }
}

void global_consistency(int id)
{
    int pid = ATM_buf.request.client_pid;
    int balance = 0;
    //cout<<"Hi there, this is master!"<<endl;
    msgctl(msqid[id], IPC_STAT, &qinfo);
    time_t tstamp = qinfo.msg_rtime;
    
    // Get shmids
    shm_create();
    char *shm;
    for(int i = 0; i< ATM_num; i++)
    {    
        // Attach shared memory to use
        data = (shared_mem *)shmat(shmid[i], NULL, 0);
        shm = (char *)data;
        if (shm == (char *) -1) 
        {
            perror("shmat");
            exit(1);
        }
        else
        {
            //cout<<shm<<endl;
            //data = (shared_mem *)shm;
            //cout<<"Data: "<<data->num<<" || "<<data->balance<<" || "<<data->time_stamp<<" || "<<data->current_pid<<endl;
            for(int j = 0; j< data->num; j++)
            {
                //cout<<"Here master!\nIn i = "<<i<<"\nPid: "<<pid<<" : "<<"Current_pid: "<<(data->current_session[j]).client_pid<<" || "<<(data->current_session[j]).dep_or_withdraw<<endl;
                if( pid == (data->current_session[j]).client_pid )
                {
                    if( (data->current_session[j]).dep_or_withdraw )
                    {
                        balance -= (data->current_session[j]).amnt;
                    }
                    else
                    {
                        balance += (data->current_session[j]).amnt;
                    }
                }
            }
        }
        // Detach the shared memory
        if( shmdt(data) == -1)
        {
            perror("shmdt : ");
            exit(1);
        }
        //else cout<<"Detached Shared Mem "<< i <<" from Local Mem Seg"<<endl;
    }

    //cout<<"Balance : "<<balance<<endl;
    
    if ((shm = (char *)shmat(shmid[id], NULL, 0)) == (char *) -1) 
    {
        perror("shmat");
        exit(1);
    }
    else
    {   
        //cout<<"Master: I did come here after all"<<endl;
        data = (shared_mem *)shm;
        data->balance = balance;
        data->time_stamp = tstamp;
        for(int i = 0; i< clients.size(); i++)
        {   
            if(pid == clients[i].pid)
            {
                clients[i].balance = balance;
                ATM_buf.request.balance = clients[i].balance;
            }
        }
    }
}

void msg_qcreate()
{
    //printf("Message queues for ATM to Master creation\n");
    for(int i = 0; i< ATM_num; i++)
    {
        if ((msqid[i] = msgget(ATM_keys[i][0], IPC_CREAT | 0666 )) < 0) 
        {
            perror("msgget error :");
            exit(1);      
        }
        // else 
        //     printf("Message queue %d created : %d\n",i,msqid[i]);
    }
}

void initialize()
{
    for(int i = 0; i< ATM_num; i++)
    {
        ATM_keys[i][0] = 10 + i;       // Message Queue IDs for ATM to Master communtication
        ATM_keys[i][1] = 100 + i;     // Semaphore IDs for locking between ATM and Client
        ATM_keys[i][2] = 500 + i;     // Shared Memory IDs
        ATM_keys[i][3] = 1000 + i;   // Message Queue IDs for ATM to Client communication
    }
    ATM_buf.mtype = 1;
}

void put_ATMfile_data()
{
    ATM_Locator = get_working_path() + "/ATMLocator.txt";
    fstream f;
    f.open(ATM_Locator.c_str(), ios::out);
    f.close();
    f.open(ATM_Locator.c_str(), ios::out | ios::app);
    int i;
    for(i = 0; i < ATM_num - 1; i++)
    {
        f<<i+1<<" "<<ATM_keys[i][3]<<" "<<ATM_keys[i][1]<<" "<<ATM_keys[i][2]<<endl;
    }
    f<<i+1<<" "<<ATM_keys[i][3]<<" "<<ATM_keys[i][1]<<" "<<ATM_keys[i][2];
    f.close();
}

string get_working_path()
{
    char temp[256];
    return ( getcwd(temp, 256) ? string( temp ) : string("") );
}

// MAIN
int main(int argc , char *argv[])
{
    if(argc < 2)
    {
        printf("Problem, enter as:  <.out file> <number of ATMs>\n");
        exit(0);
    }
    ATM_num = atoi(argv[1]);

    //Signal Handler Setup
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

    // Initialize
    initialize();

    // Generate ATMs and ATMLocator File
    generate();


    // int shmid;
    // key_t key;
    // char *shm, *s;

    // /*
    //  * We'll name our shared memory segment
    //  * "5678".
    //  */
    // key = 5678;

    // /*
    //  * Create the segment.
    //  */
    // if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
    //     perror("shmget");
    //     exit(1);
    // }

    // /*
    //  * Now we attach the segment to our data space.
    //  */
    // if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    //     perror("shmat");
    //     exit(1);
    // }

    // /*
    //  * Now put some things into the memory for the
    //  * other process to read.
    //  */
    // s = shm;

    // for (c = 'a'; c <= 'z'; c++)
    //     *s++ = c;
    // *s = NULL;

    // /*
    //  * Finally, we wait until the other process 
    //  * changes the first character of our memory
    //  * to '*', indicating that it has read what 
    //  * we put there.
    //  */
    // while (*shm != '*')
    //     sleep(1);
    for(int i = 0; i < ATM_num; i++)
    {
        msgctl(msqid[i], IPC_RMID, NULL);
        //shmctl(shmid[i], IPC_RMID, NULL);
    }
    return 0;
}