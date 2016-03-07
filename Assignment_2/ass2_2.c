/* 
S Raagpriya 13CS10043
Mayank Roy 13CS30021
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <math.h> 


int search(int c,int array[],int no)
{
  int i;
  for(i=0;i < no;i++)
  {
    if(array[i] == c)
      return(-1);
  }
  return(1);
}

int check_prime(int a)
{
   int c;
    for ( c = 2 ; c <= (int) sqrt (a) ; c++ )
    { 
      if ( a%c == 0 )
        return 0;
   
    if ( c == (int) sqrt (a) )
      return 1;
    }  
}

void main()
{
  int n,k;
	scanf("%d %d",&n,&k);
   
  int numprime = 0 , primearr[n];
  int i,l;
  //printf("%d", (int)(sizeof(primearr)/sizeof(int)));

  pid_t pid;
  int ptc[k][2],ctp[k][2],temp1,temp2,temp;

  for(i=0;i<k;i++)
  {   
        //emp = i;
    temp1 = pipe(ptc[i]);
    temp2 = pipe(ctp[i]);
        //pid[i] = fork();
  }

  for(l=0;l<k;l++)
  {
    pid = fork();
    if(pid == 0)
    {
      break;
    }
  }

  if(pid == 0)
  {
    printf("%d %d\n",n,k);
    /* Child process closes up input side of pipe */
    close(ptc[l][0]);
    /* Child process closes up output side of pipe */
    close(ctp[l][1]); 

    int available = 30001, busy = 30002,num,j,numb[k],nbyte,lol[3];
    lol[0] = 3; lol[1] = 5; lol[2] = 7;               

    /* Send "signal for available" through the output side of pipe */
    while(1)
    {
      write(ptc[l][1], &available, sizeof(available)); 
      printf("%d before sleep %d\n",n,k);
      sleep(2);
      printf("after sleep %d\n",l);
            
      for(j=0;j<k;j++)
        nbyte = read(ctp[l][0], &(numb[j]), sizeof(numb[j]));
      for(j=0;j<k;j++)
        printf("from child %d : %d\n ",l,numb[j]);
      write(ptc[l][1], &busy, sizeof(busy));

      for(j=0;j<k;j++)
      {
                    // write(ptc[l][1], &(lol[j]), sizeof(lol[j]));
        if(check_prime(numb[j]) == 1)
        write(ptc[l][1], &(numb[j]), sizeof(numb[j]));
      }
    }
  }

  else
  {
    printf("parent \n");

    for(i=0;i<k;i++)
    {
            /* Parent process closes up output side of pipe */
      close(ptc[i][1]);
            /* Parent process closes up intput side of pipe */
      close(ctp[i][0]);            

        }
        printf("parent2\n");
    

        int nbytes,c,sig,j;
        


        c =0;
        srand(time(NULL));

        printf("parent3\n");

        while(1)
        {

           printf("parent4\n");
           nbytes = read(ptc[c][0], &sig, sizeof(sig));
           printf("%d %d\n",c,sig);
           printf("parent5\n");
  
           if(sig == 30001)
           {
               for(j=0;j<k;j++)
               {
                   int r = rand() % 30000; 
                     
                   
                   printf("%d\n",r);
                   write(ctp[c][1], &r , sizeof(r));
               }
               printf("parent6\n");

              c = c + 1;
              if(c == k)
                 c = 0;
               printf( "c = %d\n",c); 
               continue;
           }

           if(sig == 30002) 
           {
                c = c+1;
                if(c == k)
                  c = 0;
                continue;
            }

           else
           {
              printf("Else loop\n");
              int result;
              result = search(sig,primearr,numprime);
              if(result == -1)
                 continue;
              else
              {
                 if(result == 1)
                   {
                     
                     primearr[numprime] = sig;
                     numprime = ++numprime;
                   }
              }

           }

            if(numprime == n)
            {
                int w;
                for(w=0;w < n ; w++)
                {
                    printf("%d\n",primearr[w]);
                }
                break;
               
            }
        } 
       killpg(0, SIGKILL);  
    }

}

   


