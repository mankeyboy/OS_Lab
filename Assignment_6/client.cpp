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

typedef struct ATM_data
{
    int ATM_id;
    key_t msqkey;
    int msqid;
    key_t semkey;
    int semid;
    key_t shmkey;
    int shmid;
}ATM;

// Global Variables

message_buf buf;
vector <ATM> ATM_List;
ATM current;
string ATM_Locator, req;
int ATM_num;
struct sembuf sop;

// Function Declarations

void entry();
void connect(int);
void initialize();
string get_working_path();
void inside_ATM(int);

// Function Definitions

void inside_ATM(int id)
{
    cout<<"Client : ";
    cin>>req;

    size_t found;
    
    if(req.find("ADD")!= string::npos)
    {
        found = req.find(":");
        string temp_amnt = req.substr(found+1, req.size());
        //cout<<found <<" at loc that || ";
        req.erase(found, req.size());
        //cout<<req<<" after erasing and amnt " <<temp_amnt<<endl;
       
        buf.request.req_type = 3;
        //Implement This
        int amnt = atoi(temp_amnt.c_str());
        buf.request.transaction_amnt = amnt;
        buf.request.ret_type = -1;
        if (msgsnd(current.msqid, &buf, sizeof(struct msg), 0) < 0) 
        {
            cout<<sizeof(buf)<<" : "<<sizeof(buf.request)<<" : "<<current.msqid<<" : "<<buf.request.client_pid<<" : "<<sizeof(long)<<endl;
            perror("msgsnd");
            exit(1);
        }
        else 
        {
            // printf("Message Sent to \"%ld\"  \n", buf.mtype);
            // printf("Successfully inserted in queue %d\n",current.msqid );
        }

        if(msgrcv(current.msqid, &buf,sizeof(struct msg),1,0)!=-1)
        {
            if(buf.request.ret_type == 300)
            {
                cout<<"Rs "<<amnt<<" added to account "<<getpid()<<endl;
                inside_ATM(id);
            }                
        }
        else
        {
            cout<<"Transaction Failed!\nPlease Check Balance to verify!\n";
            inside_ATM(id);
        }
    }
    else if(req.find("WITHDRAW")!= string::npos)
    {
        found = req.find(":");
        string temp_amnt = req.substr(found+1, req.size());
        //cout<<found <<" at loc that || ";
        req.erase(found, req.size());
        //cout<<req<<" after erasing and amnt " <<temp_amnt<<endl;
        buf.request.req_type = 2;
        //cout<<"Withdrawing"<<endl;
        //Implement This
        int amnt = atoi(temp_amnt.c_str());
        buf.request.transaction_amnt = amnt;
        buf.request.ret_type = -1;
        if (msgsnd(current.msqid, &buf, sizeof(struct msg), 0) < 0) 
        {
            cout<<sizeof(buf)<<" : "<<sizeof(buf.request)<<" : "<<current.msqid<<" : "<<buf.request.client_pid<<" : "<<sizeof(long)<<endl;
            perror("msgsnd");
            exit(1);
        }
        else 
        {
            // printf("Message Sent to \"%ld\"  \n", buf.mtype);
            // printf("Successfully inserted in queue %d\n",current.msqid );
        }

        if(msgrcv(current.msqid, &buf,sizeof(struct msg),1,0)!=-1)
        {
            if(buf.request.ret_type == 199)
            {
                cout<<"Balance Insufficient"<<endl;
                inside_ATM(id);
            }
            else if(buf.request.ret_type == 200)
            {
                cout<<"Withdraw Successful"<<endl;
                inside_ATM(id);
            }                
        }
        else
        {
            cout<<"Transaction Failed!\nPlease Check Balance to verify!\n";
            inside_ATM(id);
        }
    }
    else if(!strcmp(req.c_str(), "VIEW"))
    {
        buf.request.req_type = 4;
        //Implement This
        buf.request.balance = 0;
        buf.request.transaction_amnt = 0;
        buf.request.ret_type = -1;
        if (msgsnd(current.msqid, &buf, sizeof(struct msg), 0) < 0) 
        {
            cout<<sizeof(buf)<<" : "<<sizeof(buf.request)<<" : "<<current.msqid<<" : "<<buf.request.client_pid<<" : "<<sizeof(long)<<endl;
            perror("msgsnd");
            exit(1);
        }
        else 
        {
            // printf("Message Sent to \"%ld\"  \n", buf.mtype);
            // printf("Successfully inserted in queue %d\n",current.msqid );
        }
        //Receiving View
        if(msgrcv(current.msqid, &buf,sizeof(struct msg),1,0)!=-1)
        {
            //cout<<"Returned req: "<<buf.request.ret_type<<endl;
            if(buf.request.ret_type == 400)
            {
                cout<<"Available Balance: "<<buf.request.balance<<endl;
            }
            else if(buf.request.ret_type == 399)
            {
                cout<<"Master busy! Try again later!\n";
            }
            inside_ATM(id);
        }
        else
        {
            cout<<"Balance Retrieval Unsuccessful!\n";
            inside_ATM(id);
        }

    }
    else if(!strcmp(req.c_str(), "LEAVE"))
    {
        buf.request.req_type = 5;
        buf.request.balance = 0;
        buf.request.transaction_amnt = 0;
        buf.request.ret_type = -1;
        if (msgsnd(current.msqid, &buf, sizeof(struct msg), 0) < 0) 
        {
            cout<<sizeof(buf)<<" : "<<sizeof(buf.request)<<" : "<<current.msqid<<" : "<<buf.request.client_pid<<" : "<<sizeof(long)<<endl;
            perror("msgsnd");
            exit(1);
        }
        else 
        {
            // printf("Message Sent to \"%ld\"  \n", buf.mtype);
            // printf("Successfully inserted in queue %d\n",current.msqid );
        }
        //Receiving LEAVE
        if(msgrcv(current.msqid, &buf,sizeof(struct msg),1,0)!=-1)
        {
            if(buf.request.ret_type == 500)
            {
                sop.sem_op = 1;                                                         // LEAVING ATM and releasing access lock
                semop(current.semid, &sop, 1);
                cout<<"Left ATM"<<id<<endl;
                entry();
            }
        }
        else
        {
            cout<<"Unable to leave!\nTry again!\n";
            inside_ATM(id);
        }
        
    }
    else
    {
        cout<<"Wrong Option entered\nTry Again!\n";
        inside_ATM(id);
    }
}

void connect(int id)
{
    current = ATM_List[id-1];
    if(ATM_List[id-1].semid == -1)
    {
        if((current.semid = semget(current.semkey, 1, IPC_CREAT|0666)) < 0)
        {
            perror("Semaphore for ATM creation failed");
            exit(1);
        }
        ATM_List[id-1].semid = current.semid;
         // else 
         //    printf("ATM Sem accessed : %d\n",current.semid);  
    }

    sop.sem_num = 0;    
    sop.sem_flg = IPC_NOWAIT;
    sop.sem_op = -1;
    // t.tv_sec = 2;
    // t.tv_nsec = 0;
    if(semop(current.semid, &sop, 1) == -1) // Requesting access for ATM
    {
        cout<<"ATM"<<id<<" occupied\n";
        entry();
    }
    else                                                    // Access Granted
    {
        if(ATM_List[id-1].msqid == -1)
        {
            if((current.msqid = msgget(current.msqkey, IPC_CREAT|0666 )) < 0) 
            {
                perror("msgget");
                exit(1);      
            }
            else 
                printf("Message queue for ATM accessed : %d\n",current.msqid);
            ATM_List[id-1].msqid = current.msqid;
        }
        // Requesting Join
        buf.mtype = 1;
        buf.request.balance = 0;
        buf.request.req_type = 1;
        buf.request.client_pid = getpid();
        buf.request.transaction_amnt = 0;
        buf.request.ret_type = -1;
        if (msgsnd(current.msqid, &buf, sizeof(struct msg), 0) < 0) 
        {
            cout<<sizeof(buf)<<" : "<<sizeof(buf.request)<<" : "<<current.msqid<<" : "<<buf.request.client_pid<<" : "<<sizeof(long)<<endl;
            perror("msgsnd");
            exit(1);
        }
        else 
        {
            printf("Message Sent to \"%ld\"  \n", buf.mtype);
            printf("Successfully inserted in queue %d\n",current.msqid );
        }
        //Receiving Join
        if(msgrcv(current.msqid, &buf,sizeof(struct msg),1,0)!=-1)
        {
            if(buf.request.ret_type == 100)
            {
                cout<<"Inside ATM"<<id<<" \nWelcome Client "<<getpid()<<endl<<endl;
                cout<<"Options:\n\nLEAVE\nWITHDRAW:<amnt>\nADD:<amnt>\nVIEW\n\n";
                inside_ATM(id);
            }
            else if(buf.request.ret_type == 101)
            {
                cout<<"Inside ATM"<<id<<" \nWelcome New Client "<<getpid()<<endl<<endl;
                cout<<"Options:\n\nLEAVE\nWITHDRAW:<amnt>\nADD:<amnt>\nVIEW\n\n";
                inside_ATM(id);
            }
            else
            {
                cout<<buf.request.ret_type<< " : ";
                cout<<"Value Error\nTrying Again!\n";
                connect(id);
            }
        }
        else
        {
            cout<<"Trying Again!\n";
            connect(id);
        }       
    }
}

void entry()
{
    cout<<"\nClient : ENTER ATM";
    int id;
    cin>>id;
    if(id > 0 && id <= ATM_num)
    {
        connect(id);
    }
    else 
    {
        cout<<"\nExit Value < 0 or > ATM_num received\nExiting!\n";
        exit(0);
    }

}


void initialize()
{
    ATM_Locator = get_working_path() + "/ATMLocator.txt";
    fstream f;
    f.open(ATM_Locator.c_str(), ios::in);
    char temp[10];
    ATM tempdata;
    int i = 0;
    while(!f.eof())
    {
        f>>temp;
        //cout<<temp<<endl;
        tempdata.ATM_id = atoi(temp);
        f>>temp;
        //cout<<temp<<endl;
        tempdata.msqkey = atoi(temp);
        tempdata.msqid = -1;
        f>>temp;
        //cout<<temp<<endl;
        tempdata.semkey = atoi(temp);
        tempdata.semid = -1;
        f>>temp;
        //cout<<temp<<endl;
        tempdata.shmkey = atoi(temp);
        tempdata.shmid = -1;
        ATM_List.push_back(tempdata);   
        i++;     
    }
    ATM_num = i;
    buf.mtype = (long)1;
    buf.request.client_pid = -1;
    buf.request.transaction_amnt = 0;
    buf.request.req_type = -1;
    buf.request.ret_type = -1;
    //cout<<ATM_List[4].
}

string get_working_path()
{
    char temp[256];
    return ( getcwd(temp, 256) ? std::string( temp ) : std::string("") );
}

// Main Function
int main()
{
    initialize();
    entry();
    return 0;
}