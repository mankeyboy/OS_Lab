/*

13CS10043 S Raagapriya
13CS30021 Mayank Roy
*/

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


void main()
{   
	int tim,n,i;
	pid_t pid;
    int itr,prio,sleep_tim;
    int prob;
    n = 4;
    char * command;
    command = (char*)malloc(300*sizeof(char));
    //strcpy(command, "gnome-terminal -x sh -c './p ; cat'");

    for(i = 0;i<n;i++)
	{
		  pid = fork();
		  if(pid == 0)
		  {
		  	  if(i < 2)
		  	  {
			  	 itr = 100;
			  	 prio = 10;
			  	 sleep_tim = 1;
			  	 prob = 30;
		  	 }
		  	 else
		  	 {
                 itr = 40;
			  	 prio = 5;
			  	 sleep_tim = 3;
			  	 prob = 70;
		  	 }	
		  	 printf("itr = %d\n",itr);
		  	 sprintf(command,"gnome-terminal -x sh -c './p %d %d %d %d %d; cat'",itr,prob,sleep_tim,prio,prio);
		  	 printf("%s\n",command);
		     system(command);
		     break;
	      }
		  else
		    sleep(1);
	}
	printf("i am here\n");
}