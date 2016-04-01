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

int ID, ATM_semid, master_msqid, client_msqid, shmid, ATM_num, ATM_shmids[ATMMAX];
key_t ATM_sem, master_msq, client_msq, shmkey, s_keys[ATMMAX];
shared_mem *localdata, *data;
vector <client> clients;
string ATM_Locator;
struct sembuf client_sop, master_sop;
message_buf client_buf, master_buf;
struct msqid_ds qinfo;
client current;
long bal;

// Function Declarations

void connect();
string get_working_path();
void initialize();
void shared_mem_init();
void shared_mem_push(transaction);
int local_consistency();
void get_ATM_shmids();
int global_consistency();

// Function Definitions

int global_consistency()
{
    master_buf.request = client_buf.request;
    // cout<<"Yo, here too be views"<<endl;
    if (msgsnd(master_msqid, &master_buf, sizeof(struct msg), 0) < 0) 
    {
        //cout<<"Did I fail\n";
        client_buf.request.ret_type = 399;                  // 399 Status for busy master
        if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
        {
            perror("msgsnd");
            exit(1);
        }
        else connect();
    }
    else 
    {
        // cout<<"Did I succeed\n";
        // printf("Message Sent to \"%ld\"  \n ", master_buf.mtype);
        // printf("Successfully inserted in queue %d\n",master_msqid );
        if(msgrcv(master_msqid, &master_buf,sizeof(struct msg),1,0)!=-1)
        {
            // cout<<"Did I receive the news"<<endl;
            // cout<<"Returned client req: "<<client_buf.request.ret_type<<" || Returned master req: "<<master_buf.request.ret_type<<endl;
            if(master_buf.request.ret_type == 400 && master_buf.request.balance != -1)
            {
                // cout<<"Wow, I really succeeded"<<endl;
                current.balance = master_buf.request.balance;
                current.is_checked = 2;
                // client_buf.request.balance = master_buf.request.balance;
                // client_buf.request.client_pid = master_buf.request.client_pid;
                // client_buf.request.transaction_amnt = master_buf.request.transaction_amnt;
                // client_buf.request.req_type = master_buf.request.req_type;
                // client_buf.request.ret_type = master_buf.request.ret_type;
                client_buf.request = master_buf.request;
                // cout<<"Returned client req: "<<client_buf.request.ret_type<<" || Returned master req: "<<master_buf.request.ret_type<<endl;
                // if(local_consistency() == 0 )
                // {
                //     client_buf.request.balance = bal;
                //     return 1;
                // }
                // else
                // {
                //     client_buf.request.balance = bal;
                    return 1;
                // }
            }
            else return 0;
        }
    }
    
}

void get_ATM_shmids()
{
    ATM_Locator = get_working_path() + "/ATMLocator.txt";
    fstream f;
    f.open(ATM_Locator.c_str(), ios::in);
    char temp[10];
    int i = 0;
    while(!f.eof() && i < ATM_num)
    {
        f>>temp;
        f>>temp;
        f>>temp;
        f>>temp;
        //cout<<temp<<endl;
        s_keys[i] = atoi(temp);
        ATM_shmids[i] = -1;
        if(s_keys[i] != shmkey)
        {
            if ((ATM_shmids[i] = shmget(s_keys[i], SHMSZ, IPC_CREAT | 0666)) < 0) 
            {
                printf("In ATM %d : ", i);
                perror("shmget : ");
                exit(1);
            }
        }
        else ATM_shmids[i] = shmid; 
        i++;     
    }
}

int local_consistency()
{
    int trans_amnt = client_buf.request.transaction_amnt;
    int pid = client_buf.request.client_pid;
    int balance = 0;
    //msgctl(client_msqid, IPC_STAT, &qinfo);
    //time_t tstamp = qinfo.msg_rtime;

    // Attach shared memory to use
    char *shm;
    for(int i = 0; i< ATM_num; i++)
    {
        if ((shm = (char *)shmat(ATM_shmids[i], NULL, 0)) == (char *) -1) 
        {
            perror("shmat");
            exit(1);
        }
        else
        {
            //cout<<shm<<endl;
            data = (shared_mem *)shm;
            for(int j = 0; j< data->num; j++)
            {
                //cout<<"Here atm!\nIn i = "<<i<<"\nPid: "<<pid<<" : "<<"Current_pid: "<<(data->current_session[j]).client_pid<<endl;
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
        if( shmdt(shm) == -1)
        {
            perror("shmdt : ");
            exit(1);
        }
        // else cout<<"Detached Shared Mem "<< i <<" from Local Mem Seg"<<endl;
    }
    // cout<<"Balance : "<<balance<<" || Transaction Amnt : "<<trans_amnt<<endl;
    bal = balance;
    if(balance >= trans_amnt)
    {
        current.balance = balance;
        return 1;
    }
    else return 0;
}

void shared_mem_init()
{
    localdata->time_stamp = -1;
    localdata->current_pid = -1;
    localdata->balance = -1;
    localdata->num = 0;
    for(int i = 0; i< TRANSSZ; i++)
    {
        (localdata->current_session[i]).t_trans = -1;
        (localdata->current_session[i]).client_pid = -1;
        (localdata->current_session[i]).amnt = -1;
        (localdata->current_session[i]).dep_or_withdraw = -1;
    }
}

void shared_mem_push(transaction t)
{
    localdata->time_stamp = t.t_trans;
    localdata->current_pid = t.client_pid;
    localdata->current_session[localdata->num] = t;
    localdata->num++;
}

void connect()
{
    //cout<<sizeof(request_data)<<" : "<<client_msqid<<endl;
    if(msgrcv(client_msqid, &client_buf,sizeof(struct msg),1,0)!=-1)
    {
        //cout<<"Request received!\n";
        cout<<client_buf.request.req_type<<" : "<<client_buf.request.client_pid<<endl;
        switch(client_buf.request.req_type)
        {
            case 1:                                                                        // JOIN
            {
                master_buf.request = client_buf.request;
                //cout<<"Sending Req to Master for client "<<client_buf.request.client_pid<<endl;
                
                if (msgsnd(master_msqid, &master_buf, sizeof(struct msg), 0) < 0) 
                {
                    perror("msgsnd");
                    exit(1);
                }
                else 
                {
                    // printf("Message Sent to \"%ld\"  \n ", master_buf.mtype);
                    // printf("Successfully inserted in queue %d\n",master_msqid );

                }
                if(msgrcv(master_msqid, &master_buf,sizeof(struct msg),1,0)!=-1)
                {
                    if(master_buf.request.ret_type == 100 || master_buf.request.ret_type == 101)
                    {
                        //cout<<"Inside ATM"<<id<<" \n Welcome Client "<<getpid()<<endl;
                        //inside_ATM(id);
                        cout<<"Ready"<<endl;
                        current.trans_history.erase(current.trans_history.begin(), current.trans_history.end());
                        current.pid = client_buf.request.client_pid;
                        current.balance = master_buf.request.balance;
                        current.is_checked = 1;
                        if(master_buf.request.ret_type == 101)
                        {
                            cout<<"Welcome New Client "<<client_buf.request.client_pid<<endl;
                            clients.push_back(current);
                        }
                        else if(master_buf.request.ret_type == 100)
                        {
                            cout<<"Welcome Client "<<client_buf.request.client_pid<<endl;
                            int i;
                            for(i = 0; i< clients.size(); i++)
                            {
                                if(current.pid == clients[i].pid)
                                    break;
                            }
                            if(i == clients.size())
                            {
                                clients.push_back(current);
                            }
                        }
                        client_buf.request = master_buf.request;                  // 100 Status for successful joining
                        if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
                        {
                            perror("msgsnd");
                            exit(1);
                        }
                        else 
                        {
                            // printf("Message Sent to \"%ld\"  \n ", client_buf.mtype);
                            // printf("Successfully inserted in queue %d\n",client_msqid );
                            connect();
                        }
                    }
                    else
                    {
                        //cout<<master_buf.request.ret_type<< "\n";
                        //cout<<"Value Error\nTrying Again!\n";
                        //connect(id);
                    }
                }
                break;
            }

            case 2:                                       // WITHDRAW
            {
                if(local_consistency())
                {
                    client temp;
                    current.pid = client_buf.request.client_pid;
                    current.is_checked = 1;

                    transaction temp_t;
                    temp_t.client_pid = current.pid;
                    temp_t.amnt = client_buf.request.transaction_amnt;
                    temp_t.dep_or_withdraw = 1;
                    msgctl(client_msqid, IPC_STAT, &qinfo);
                    temp_t.t_trans = qinfo.msg_rtime;
                    current.trans_history.push_back(temp_t);
                    // Attach shared memory to use
                    char *shm;
                    if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) 
                    {
                        perror("shmat");
                        exit(1);
                    }
                    else
                    {
                        localdata = (shared_mem *)shm;
                        shared_mem_push(temp_t);
                        //cout<<"This is happening\n";
                        //cout<<temp_t.client_pid<<" : "<<temp_t.amnt<<" : "<<temp_t.t_trans<<endl;

                    }
                    // Detach the shared memory
                    if( shmdt(shm) == -1)
                    {
                        perror("shmdt : ");
                        exit(1);
                    }
                    // else cout<<"Detached from Local Mem Seg"<<endl;

                    for(int i = 0; i<clients.size(); i++)
                    {
                        if(current.pid == clients[i].pid)
                        {
                            // cout<<"Came here and got stuck in Withdrawl!!!!"<<endl;
                            clients[i].trans_history.insert(clients[i].trans_history.end(), current.trans_history.end()-1, current.trans_history.end());
                            // cout<<"Reached here!!"<<endl;
                            client_buf.request.ret_type = 200;                  // 200 Status for successful withdraw
                            if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
                            {
                                perror("msgsnd");
                                exit(1);
                            }
                            else 
                            {
                                //printf("Message Sent to \"%ld\"  \n  ", client_buf.mtype);
                                //printf("Successfully inserted in queue %d\n",client_msqid );
                                connect();
                            }
                        }
                    }
                }
                else
                {
                    client_buf.request.ret_type = 199;                  // 199 Status for unsuccessful withdrawal
                    if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
                    {
                        perror("msgsnd");
                        exit(1);
                    }
                    else 
                    {
                        //printf("Message Sent to \"%ld\"  \n  ", client_buf.mtype);
                        //printf("Successfully inserted in queue %d\n",client_msqid );
                        connect();
                    }
                }
                break;
            }

            case 3:                                     // ADD
            {
                current.pid = client_buf.request.client_pid;
                current.is_checked = 0;

                transaction temp_t;
                temp_t.client_pid = current.pid;
                temp_t.amnt = client_buf.request.transaction_amnt;
                temp_t.dep_or_withdraw = 0;
                msgctl(client_msqid, IPC_STAT, &qinfo);
                temp_t.t_trans = qinfo.msg_rtime;
                current.trans_history.push_back(temp_t);
                // Attach shared memory to use
                char *shm;
                if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) 
                {
                    perror("shmat");
                    exit(1);
                }
                else
                {
                    localdata = (shared_mem *)shm;
                    shared_mem_push(temp_t);
                    //cout<<"This is happening\n";
                    //cout<<temp_t.client_pid<<" : "<<temp_t.amnt<<" : "<<temp_t.t_trans<<endl;

                }
                // Detach the shared memory
                if( shmdt(shm) == -1)
                {
                    perror("shmdt : ");
                    exit(1);
                }
                // else cout<<"Detached from Local Mem Seg"<<endl;
                //cout<<clients.size()<<" yoyo\n";
                for(int i = 0; i<clients.size(); i++)
                {
                    if(current.pid == clients[i].pid)
                    {
                        // cout<<"Came here and got stuck!!!!"<<endl;
                        clients[i].trans_history.insert(clients[i].trans_history.end(), current.trans_history.end()-1, current.trans_history.end());
                        //cout<<"Reached here!!"<<endl;
                        client_buf.request.ret_type = 300;                  // 300 Status for successful add
                        if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
                        {
                            perror("msgsnd");
                            exit(1);
                        }
                        else 
                        {
                            //printf("Message Sent to \"%ld\"  \n  ", client_buf.mtype);
                            //printf("Successfully inserted in queue %d\n",client_msqid );
                            connect();
                        }
                    }
                }
                break;
            }
        
            case 4:                                        // VIEW
            {
                // cout<<"Here be view!"<<endl;
                if(global_consistency() && current.is_checked==2)
                {
                    //client_buf.request.ret_type = 400;                  // 400 Status for successful  view
                    if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
                    {
                        perror("msgsnd");
                        exit(1);
                    }
                    else
                    {
                        //printf("Message Sent to \"%ld\"  \n  ", client_buf.mtype);
                        //printf("Successfully inserted in queue %d\n",client_msqid );
                        connect();
                    }
                }
                break;
            }  
            case 5:                                          // LEAVE
            {
                client_buf.request.ret_type = 500;                  // 500 Status for successful leave
                if (msgsnd(client_msqid, &client_buf, sizeof(struct msg), 0) < 0) 
                {
                    perror("msgsnd");
                    exit(1);
                }
                else 
                {
                    //printf("Message Sent to \"%ld\"  \n ", client_buf.mtype);
                    //printf("Successfully inserted in queue %d\n",client_msqid );
                    cout<<"Client "<<client_buf.request.client_pid<<" left!\n";
                    connect();
                }
                break;
            } 
        }  
    } 
    else
    {
        perror("error in receiving : ");
        exit(1);
    } 
}

void sigint_handler(int sig)
{
    //write(0, "Ahhh! SIGINT!\n", 14);
    msgctl(client_msqid, IPC_RMID, NULL);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(ATM_semid, 0, IPC_RMID);
    //killpg(0,SIGKILL);
    exit(1);
}

void initialize()
{
    client_buf.mtype = (long)1;
    client_buf.request.client_pid = -1;
    client_buf.request.transaction_amnt = 0;
    client_buf.request.req_type = -1;
    client_buf.request.ret_type = -1;
    master_buf.mtype = (long)1;
    master_buf.request.client_pid = -1;
    master_buf.request.transaction_amnt = 0;
    master_buf.request.req_type = -1;
    master_buf.request.ret_type = -1;
    //printf("Message queue for ATM to Master creation\n");
    if ((master_msqid = msgget(master_msq, IPC_CREAT|0666)) < 0) 
    {
        perror("msgget error :");
        exit(1);      
    }
    // else 
    //     printf("Message queue created : %d\n",master_msqid);
   
    //printf("Message queue for ATM to Client creation\n");
    if ((client_msqid = msgget(client_msq, IPC_CREAT|0666)) < 0) 
    {
        perror("msgget error :");
        exit(1);      
    }
    // else 
    //     printf("Message queue AtC created : %d\n", client_msqid);
    
    if((ATM_semid = semget(ATM_sem, 1, IPC_CREAT|0666)) < 0)
    {
        perror("semaphore for ATM creation failed");
        exit(1);
    }
     // else 
     //    printf("ATM Sem created : %d\n",ATM_semid);  
    semctl(ATM_semid, 0, SETVAL, 1);

    if ((shmid = shmget(shmkey, SHMSZ, IPC_CREAT | 0666)) < 0) 
    {
        perror("shmget");
        exit(1);
    }   
    // else 
    //    printf("Shared memory created : %d\n",shmid);
    char *shm;
    if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) 
    {
        perror("shmat");
        exit(1);
    }
    else
    {
        localdata = (shared_mem *)shm;
        shared_mem_init();
    }
    // Detach the shared memory
    if( shmdt(shm) == -1)
    {
        perror("shmdt : ");
        exit(1);
    }
    get_ATM_shmids();
     
}

string get_working_path()
{
    char temp[256];
    return ( getcwd(temp, 256) ? std::string( temp ) : std::string("") );
}


// MAIN FUNCTION
int main(int argc, char *argv[])
{
    if(argc < 7)
    {
        printf("Incorrect format, check number of arguments\n");
        exit(0);
    }

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

    for(int i = 1; i< argc; i++)
    {
        if(i == 1)
            ID = atoi(argv[i]);
        else if(i == 2)
            master_msq = (key_t)atoi(argv[i]);
        else if(i == 3)
            ATM_sem = (key_t)atoi(argv[i]);
        else if(i == 4)
            shmkey = (key_t)atoi(argv[i]);
        else if(i == 5)
            client_msq = (key_t)atoi(argv[i]);
        else if(i == 6)
            ATM_num = atoi(argv[i]);
    }

    initialize();
    while(1)
        connect();

    
    msgctl(client_msqid, IPC_RMID, NULL);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(ATM_semid, 0, IPC_RMID);
    return 0;
}