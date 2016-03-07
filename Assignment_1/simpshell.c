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

//constants
const int MAX_ARG_LEN = 128;

//Global variables
char **env1= NULL;
char *path = NULL;
FILE *fp ;
int offset = 0;

//Function for clear screen
void clearScreen()
{
  const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO,CLEAR_SCREE_ANSI,12);
}

void setoffset()
{
  fp = fopen("Const_file.txt", "w+");
  fprintf(fp,"%d",offset) ;
  fclose(fp); 

}

//MAIN EXECUTION FUNCTION
int execute(char arg[2][128])
{
  arg[1][strlen(arg[1])-1] = '\0';
  //printf("\n yo yo %sabc, %d", arg[0], (int)strlen(arg[0]));
  
  if(strcmp(arg[0], "clear") == 0)
  {
      //printf("\nyoyo");
    if(arg[1][0] != '\0')
      printf("No Command '%s %s' found\n",arg[0], arg[1] ) ;
    else
      clearScreen();
  }
  
  else if(strcmp(arg[0], "env") == 0)
  {
    if(arg[1][0] != '\0')
      printf("No Command '%s %s' found\n",arg[0], arg[1] ) ;
    else
    {
      char **env;
      for(env=env1;*env !=0; env++)
      {
        char *thisEnv = *env;
        printf("%s\n", thisEnv);
      }
    } 
  }
  
  else if(strcmp(arg[0], "cd") == 0)
  {
    int ret;
    //printf("\n%s, %d", arg[1], (int)strlen(arg[1]));
    ret = chdir(arg[1]);        
    if(ret == -1)
    {
      perror("The following error(s) occured.");
    }
    else
    {
      getcwd(path, sizeof(path));
    }
  }
  
  else if(strcmp(arg[0], "pwd")==0)
  {
    getcwd(path, sizeof(path));
    printf("%s\n",path);
  }
  
  else if(strcmp(arg[0], "mkdir")==0)
  {
    int ret;
    ret = mkdir(arg[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(ret == -1)
      perror("The following error(s) occured.");
  }

  else if(strcmp(arg[0], "rmdir")== 0)
  {
    int ret;
    ret = rmdir(arg[1]);
    if(ret == -1)
      perror("The following error(s) occured.");
  }

  else if(strcmp(arg[0], "ls") == 0) 
  {
    if(arg[1][0] != '\0' && arg[1][0] == '-' && arg[1][1] == 'l')
    {
      struct stat fileStat;
      DIR *dp;
      struct dirent *sd; 
      //char path[1024];
      size_t ss;
      //getcwd(path,sizeof(path));
      dp = opendir(path);
      if(stat(path,&fileStat) < 0)    
        return 1;

      while((sd=readdir(dp))!=NULL)
      {
        char temp1[1024];
        int i=0;
        int l2=strlen(path);
        for(i=0;i<=l2;i++)
              temp1[i]=path[i];
            
        for(i=0;i<=strlen(sd->d_name);i++)
          temp1[i]=sd->d_name[i];
        
        //printf("%s..%s\n",temp1,sd->d_name);
        if(stat(temp1,&fileStat) < 0)    
          continue;
        struct passwd *pwuser;
        struct group *grpname;
        if(NULL==(pwuser=getpwuid(fileStat.st_uid)))
        {
          perror("shell");
          continue;
        }
        if(NULL==(grpname=getgrgid(fileStat.st_gid)))
        {
          perror("shell");
          continue;
        }
        
        printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
        printf("  %d",(int)fileStat.st_nlink);
        printf("  %s",pwuser->pw_name);
        printf("  %s",grpname->gr_name);
        printf("  %ld",fileStat.st_size);
        printf("   %s",ctime(&fileStat.st_atim.tv_sec));
        printf("   %s\n",sd->d_name);
      }
          
    }
    
    else if(arg[1][0] =='\0')
    {
      DIR *dir;
      struct dirent *ent;
      if ((dir = opendir (path)) != NULL) 
      {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) 
              printf ("%s\t", ent->d_name);
        closedir (dir);
      } 
      else perror ("");
      printf("\n");
      
    }
      
    else printf("No Command '%s %s' found\n", arg[0], arg[1]) ;      
  }

  else if(strcmp(arg[0],"history") == 0)
  {
    int num = 0, off;
    int isnotvalid = 0;
    char *historyLine = NULL;
    size_t len; 
    ssize_t read;

    if(arg[1][0] != '\0')
    {
      int i = 0;
      for( ;arg[1][i] !='\0';i++)
      {
        if(arg[1][i] <'0' || arg[1][i] > '9')
        {
          printf("No Command '%s %s' found\n", arg[0], arg[1] ) ;
          isnotvalid = 1;
          break;
        }
        else
          num = num*10 + (arg[1][i] -'0');
      }
      if(isnotvalid) return -1;

      fp = fopen("History_file.txt","r");

      off = 0;
      do
      {
        while ((read = getline(&historyLine, &len, fp)) != -1) 
        {
          for(int s = 0; s< strlen(historyLine); s++)
          {
            if(historyLine[s]!= ' ')
            {
              off = off*10 + (historyLine[s] -'0');
            }
            else break;
          }
          if(offset-off <= num)
            printf("%s", historyLine);
          off++;
        }
        //fscanf(fp,"%d %s", &off,command);
        
      }
      while( off < offset && !feof(fp));
    }
    else
    {
      fp = fopen("History_file.txt","r");
       
      while ((read = getline(&historyLine, &len, fp)) != -1)
              printf("%s", historyLine);
            
    }
  }
    
  else if(strcmp(arg[0], "exit")==0)
  {
    setoffset(); 
    exit(0) ;
  }

  else
  {
    int child,ret,id ;
    char c[256];
    strcpy(c,"./");
    strcat(c , arg[0]);
    int len_arg = strlen(arg[1]);
    if(fork() == 0)
    {
      if(arg[1][len_arg-1] == '&')
        arg[1][len_arg-1] = '\0' ;
      ret = execlp(arg[0], arg[0], arg[1] , (char*)NULL);
      if(ret == -1)
      {
        ret = 0 ;
        ret = execl(c,arg[0],arg[1],(char *)NULL);
        if(ret == -1)
          perror("The following error(s) occured in the current directory.");
        else
        {
          getcwd(c,sizeof(c));
          printf("%s>",c);
        } 

        }
        else
        {
          getcwd(c,sizeof(c));
          printf("%s>",c);
        } 
      }
      else
      {
        if(arg[1][len_arg-1] != '&')
          wait(&child);
      }
  }
    
  return 0;
}

//Breaking Up arguments
void split_line(char *lin, char arg[2][128])
{
  int i, j, k1;
  char temp = ' ';
  for (i = 0, j = 0, k1 = 0; i < sizeof(lin)/sizeof(char); i++ )
  {
    if(lin[i]=='\n')continue;

    if(lin[i] == ' ')
    {
      if(temp == ' ')continue;
      else if(temp != ' ')
      {
        j = 1;
        arg[0][k1]='\0';
        strcpy(arg[1], (lin+i+1));
        break;
      }
    }
    else if(!j)
    {
      temp = lin[i];
      arg[j][k1++] = temp;
    }
  }
  if(!j) arg[0][k1] = arg[1][0] = '\0';
}

//Basic read-line from user
char* read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  return line;
}

//Main loop function
void loop(void)
{
  char *line;
  char arg[2][MAX_ARG_LEN];
  int status;
  size_t size = 100*sizeof(char);

  while(1) 
  {
    //Prompt Display
    offset++;
    path = getcwd(path,size);
    printf("%s > ",path);
    
    //Read Line
    line = read_line();

    //Write line ot file for easy history recall
    fp = fopen("History_file.txt", "a+");
    fprintf(fp,"%d %s\n",offset, line) ;
    fclose(fp) ;

    
    //Parse Line and get argument array
    arg [0][0] = arg[1][0] = '\0';
    split_line(line, arg);
    //printf("Arg 1 : %s \nArg 2 : %s", arg[0], arg[1]);
    
    //Execute the user commands
    status = execute(arg);


    
    //free(line);
     
  }
}

//Getting and Setting offset for multiple executions
void getoffset()
{
  fp = fopen("Const_file.txt", "a");
  fclose(fp);

  fp = fopen("Const_file.txt", "r+");
  fscanf(fp,"%d",&offset) ;
  fclose(fp); 
}

int main(int argc, char **argv, char **envp)
{
  env1=envp;
  

  getoffset();
  // Run command loop.
  loop();
  
  return 0;

}
