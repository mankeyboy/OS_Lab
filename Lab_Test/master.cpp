/*

13CS30021
Mayank Roy
OS Lab Test 1
Machine Number 89

*/

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <bits/stdc++.h>
#include <sys/ipc.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <signal.h>
#include <errno.h>
#include <time.h>

using namespace std;

#define MAX_NODE 100
#define MAX_EDGES 100

typedef struct pipe_struct
{
    int u,v;
    int pipe_u2v[2];
    int pipe_v2u[2];
}pipe_struct;

// Global variables

int node_array[MAX_NODE][3];
pipe_struct pipes[MAX_EDGES];
int n, e;
int adj[MAX_NODE][MAX_NODE];
int graph[MAX_EDGES][MAX_EDGES];
int visited[MAX_NODE];
int level[MAX_NODE];
int lev, max_lev;

// Function Prototypes

void init();
void fork_nodes();
void iterate();
void bfs(int s, int l);

//Function Declarations

void bfs(int s,int l)
{
    int i;
    //printf("%d %d\n",level[s], sys);
    if(level[s]>max_lev)
    {
        max_lev=level[s];
        lev=s;
    }
    for(i=1;i<=n;i++)
    {
        if(adj[s][i]==1)
        {
            if(vis[i]==0)
            {
                //printf("Change here %d %d\n",i,l+1);
                level[i]=l+1;
                vis[i]=1;
                q[size++]=i;
            }
        }
    }
    if(size)
    {
        int top=q[0];
        for(i=0;i<size-1;i++)
        {
            q[i]=q[i+1];
        }
        size--;
        bfs(top,level[top]);
    }
}

void init()
{
    //Do shit
    int i = 0;
    for(i = 0; i < n; i++)
    {
        node_array[i][0] = i;
        node_array[i][1] = rand() % 100 + 1;
        node_array[i][2] = -1;		
    }
    int temp1, temp2, i;
    int u, v;
    for(i=0;i<e;i++)
    {   
        cin>> pipes[i].u>>pipes[i].v;
        pipes[i].u--;
        pipes[i].v--;
        graph[pipes[i].u][pipes[i].v] = i;
        graph[pipes[i].v][pipes[i].u] =i;
        adj[pipes[i].u][pipes[i].v] = 1; 
        adj[pipes[i].v][pipes[i].u] =1;
        temp1 = pipe(pipes[i].pipe_u2v);// For u to v
        temp2 = pipe(pipes[i].pipe_v2u);//For v to u      
    }

    visited[1]=1;
    level[1]=0;
    bfs(1,0);
    // printf("Level Max=%d Node=%d\n",max_lev,lev);
    size=0;
    for(i=0;i<=n;i++)
    {
        visited[i]=0;
        level[i]=0;
    }
    visited[lev]=1;
    level[lev]=0;
    max_lev=0;
    bfs(lev,0);
    // printf("Dia=%d Node=%d\n",max_lev,lev);
}

void fork_nodes()
{
    int i = 0;
    for(i = 0; i < n; i++)
    {
        node_array[i][2] = fork();

        if(node_array[i][2] == 0)
        {
            char c[]="./node.out";
            char **arg = (char**)malloc(sizeof(char*)*100);
            arg[0] = (char *)malloc(sizeof(char)*10);
            arg[1] = (char *)malloc(sizeof(char)*5);
            arg[2] = (char *)malloc(sizeof(char)*5);
            strcpy(arg[0], c);
            sprintf(arg[1], "%d", node_array[i][1]);
            // /* Child process closes up input side of pipe */
            //  		close(ptc[l][0]);
            //  		/* Child process closes up output side of pipe */
            //  		close(ctp[l][1]);

            execl(c, arg[0], arg[1], (char *)NULL);
        }

        else
        {
            for(int j = 0; j<e; j++)
            {
                if(pipes[j].u == i || pipes[j].v == i)
                {
                    close(pipes[j].pipe_u2v[1]);
                    close(pipes[j].pipe_v2u[0]);
                }
            }
        }
    }
}

void iterate()
{
	//Iterate over all the nodes, sending every one a kill SIGUSR1, receive their min values and then send that to all the neighbours 
	//of the nodes and when finished send the minimum so far in the pipe with a SIGUSR2 to signal the end of the iteration at which the code exits with status as min value

}

int main()
{
    srand(time(NULL));
    cin>> n >> e;

    init();

    cout << "Initial value of nodes: "; 
    for(int i = 0; i< n; i++)
    {
        cout<<"PID "<<node_array[i][2]<<" : "<<node_array[i][1]<<endl;
    }

    iterate();

    cout << "Final minimum value received from nodes: "; 
    for(int i = 0; i< n; i++)
    {
        cout<<"PID "<<node_array[i][2]<<" : "<<node_array[i][1]<<endl;
    }


    return 0;
}