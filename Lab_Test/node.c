#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int recv=0;
int neigh[100];
int pid[100];
int new_neigh[10000];
int new_pid[10000];
int new_recv=0;

int check(int pid_recv,int val)
{
	int i;
	for(i=0;i<recv;i++)
	{
		if(pid[i]==pid_recv)
			return 1;
	}
	neigh[recv]=val;
	pid[recv++]=pid_recv;
	return 0;
}

int main(int argc,char* argv[])
{
	//printf("%s %s\n",argv[1],argv[2]);
	int i;
	int val;
	int n;
	sscanf(argv[1],"%d",&val);
	sscanf(argv[2],"%d",&n);
	//printf("No of iter %d\n",n);
	int fds[100];
	int count=0;
	//printf("THESE ARE ARGS\n");
	int writefd[100];
	int readfd[100];
	int wr_count=0,rd_count=0;
	for(i=3;i<argc;i++)
	{
		//printf("%s ",argv[i]);
		sscanf(argv[i],"%d",&fds[count++]);
		if(fds[count-1]%2==0)
		{
			writefd[wr_count++]=fds[count-1];
		}
		else
		{
			readfd[rd_count++]=fds[count-1];
		}
	}
	//printf("\n");
	int it=n;
	int j,rst;
	new_neigh[new_recv]=val;
	new_pid[new_recv++]=getpid();
	neigh[recv]=val;
	pid[recv++]=getpid();
	while(it--)
	{
		for(i=0;i<wr_count;i++)
		{
			char *buf1=(char*)malloc(sizeof(char)*10);
			sprintf(buf1,"%d",new_recv);
			//printf("%s\n",buf1);
			rst=write(writefd[i],buf1,10);
			if(rst<0)
			{
				perror("write error:");
			}
			for(j=0;j<new_recv;j++)
			{
				sprintf(buf1,"%d",new_pid[j]);
				rst=write(writefd[i],buf1,10);
				//printf("%s\n",buf1);
				if(rst<0)
				{
					perror("write error:");
				}
				sprintf(buf1,"%d",new_neigh[j]);
				rst=write(writefd[i],buf1,10);
			//	printf("%s\n",buf1);
				if(rst<0)
				{
					perror("write error:");
				}
			}
		}
		new_recv=0;
		for(i=0;i<rd_count;i++)
		{
			char *read_buf=(char*)malloc(sizeof(char)*10);
			rst=read(readfd[i],read_buf,10);
			if(rst<0)
			{
				perror("read error:");
			}
			//printf("read\n");
			int x;
			sscanf(read_buf,"%d",&x);
			for(j=0;j<x;j++)
			{
				rst=read(readfd[i],read_buf,10);
				if(rst<0)
				{
					perror("read error:");
				}
				int recv_pid;
				sscanf(read_buf,"%d",&recv_pid);
				rst=read(readfd[i],read_buf,10);
				if(rst<0)
				{
					perror("write error:");
				}
				int recv_val;
				sscanf(read_buf,"%d",&recv_val);
				int present=check(recv_pid,recv_val);
				if(present==0)
				{
					new_neigh[new_recv]=recv_val;
					new_pid[new_recv++]=recv_pid;
				}
			}
		}
	}
	//printf("%d number of values received\n",recv);
	int sum=0;
	for(i=0;i<recv;i++)
	{
		//printf("%d---%d\n",neigh[i],getpid());
		sum+=neigh[i];
	}
	double mean=(sum*1.0)/(recv);
	printf("%lf is the mean\n",mean);
	exit(mean);
}