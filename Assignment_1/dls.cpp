/*
Mayank Roy 13CS30021
S Raagapriya 13CS10043
*/
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <sys/ipc.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 

using namespace std;

string filename;
const int max_size = 10000;
bool found =false;
void search_elem(int* A, int l, int r, int c);

int main(int argc, char* argv[])
{
  ifstream file;
  string input;
  int A[max_size];
  if(argc < 3 ){
    printf("Please give the appropriate details\n");
    return 0 ;
  }
  filename = argv[1];
  file.open(filename.c_str());
  if(file.is_open())
  {
    getline(file, input);
  }
  else cout<<"file doesn't exist";
  file.close();

  int temp_num = 0, j = 0;
  for(int i = 0; i<input.size(); i++)
  {
    if(input.at(i) != 32)
      temp_num = temp_num*10 + (input.at(i) - '0');
    else 
    {
      A[j]=temp_num;
      j++;
      temp_num=0; 
    }
  }
  A[j++] = temp_num;

  int c = atoi(argv[2]);
  //cin>>c;

  int size = j;
  search_elem(A, 0, size-1, c);
}

void search_elem(int* A, int l, int r, int c)
{
  if(r-l <=5)
  {
    for(int i = l; i<=r; i++)
    {
      if(A[i] == c)
      {  
        cout<<"\n\n\n     Element Found at index : "<<i<<" \n\n\n\n";
        found = true;
        killpg(0, SIGKILL);
        //Write something to kill other processes        
      }
    }
  }
  else
  {
    pid_t pid1, pid2, status;
    pid1 = fork();
    if(pid1 == 0)
    {
      search_elem (A , ((l+r)/2)+1, r, c);
      //cout<<"In child 1 process"<<endl;
    }
    else
    {
      pid2=fork();
      if(pid2==0)
      {
        search_elem (A , l, ((l+r)/2), c);
        //cout<<"In child 2 process"<<endl;
      }
      sleep(3);
      if(!found)
      {
        cout<<"\n\n\n     Element  Not Found \n\n\n\n";
        killpg(0, SIGKILL);
      }
      //waitpid(-1, NULL, 0);
      //cout<<"Not Found";
      //cout<<"In parent process"<<endl;
    }
  }
}
