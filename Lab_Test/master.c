#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
int adj[100][100];
int graph[101][101];
int edge[101][2];
int vis[100];
int level[100];
int q[100];
int size=0;
int n,m;
int max_lev;
int lev;
void bfs(int s,int l)
{
	int i;
	//printf("%d %d\n",s,level[s]);
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
				//printf("Change %d %d\n",i,l+1);
				level[i]=l+1;
				vis[i]=1;
				q[size++]=i;
			}
		}
	}
	if(size==0)
	{
		return;
	}
	else
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
int main()
{
	int i,j,a,b;
	int p[201][2];
	printf("Enter number of edges and nodes\n");
	scanf("%d%d",&n,&m);
	for(i=0;i<=n;i++)
	{
		for(j=0;j<=n;j++)
		{
			graph[i][j]=0;
			adj[i][j]=0;
		}
	}
	printf("Enter edges one by one\n");
	for(i=0;i<m;i++)
	{
		scanf("%d %d",&a,&b);
		graph[a][b]=i+1;
		graph[b][a]=i+1;
		adj[a][b]=1;adj[b][a]=1;
		pipe(p[i*2]);
		pipe(p[i*2+1]);
	}
	vis[1]=1;
	level[1]=0;
	bfs(1,0);
	//printf("Level Max=%d Node=%d\n",max_lev,lev);
	size=0;
	for(i=0;i<=n;i++)
	{
		vis[i]=0;
		level[i]=0;
	}
	max_lev=0;
	vis[lev]=1;
	level[lev]=0;
	bfs(lev,0);
	//printf("Dia=%d Node=%d\n",max_lev,lev);
	int args[2*n];
	int child[100];
	//int c_sum=0;
	for(i=1;i<=n;i++)
	{
		
		int count=0;
		//printf("adffggb %d\n",i);
		for(j=1;j<i;j++)
		{

			if(graph[i][j]!=0)
			{
				//printf("%d %d---upper\n",i,j);
				args[count++]=p[(graph[i][j]-1)*2][1];
				args[count++]=p[(graph[i][j]-1)*2+1][0];
			}
		}
		//printf("hereeee %d\n",i);
		for(j=i+1;j<=n;j++)
		{
			if(graph[i][j]!=0)
			{
				//printf("%d %d---lower\n",i,j);
				args[count++]=p[(graph[i][j]-1)*2][0];
				args[count++]=p[(graph[i][j]-1)*2+1][1];
			}
		}
		child[i]=fork();
		//printf("here %d\n",i);
		if(child[i]==0)
		{
			srand(time(NULL));
			int r1=rand()%100+1;
			printf("Random number generated is %d\n",r1);
			//c_sum+=r1;
			char **pass=(char**)malloc(sizeof(char*)*100);
			pass[0]=(char*)malloc(sizeof(char)*10);
			strcpy(pass[0],"./node");
			char r[5];
			sprintf(r,"%d",r1);
			pass[1]=(char*)malloc(sizeof(char)*5);
			strcpy(pass[1],r);
			sprintf(r,"%d",max_lev);
			//printf("%s---\n",r);
			pass[2]=(char*)malloc(sizeof(char)*5);
			strcpy(pass[2],r);
			int j;
			for(j=0;j<count;j++)
			{
				pass[j+3]=(char*)malloc(sizeof(char)*10);
				sprintf(r,"%d",args[j]);
				//printf("%s----\n",r);
				strcpy(pass[j+3],r);
			}
			pass[3+count]=NULL;
			execvp(pass[0],pass);
			//printf("Adfgfg\n");
		}
		else
		{
			sleep(1);
		}
	}
	//printf("Sum=%d Correct Mean=%lf\n",c_sum,c_sum*1.0/n);
	for(i=1;i<=n;i++)
	{
		int status;
		waitpid(child[i],&status,0);
		printf("Mean from %d %d\n",child[i],status>>8);
	}
}