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

//constants
#define MAX_ARG_LEN 255
#define MAX_ARG 16
#define MAXSIZE 100
const char history_file[] = "/home/mankeyboy/Git_maintained/codes/OS_Lab/Assignment_1/History_file.txt";
const char const_file[] = "/home/mankeyboy/Git_maintained/codes/OS_Lab/Assignment_1/Const_file.txt";

//Structs
struct stack
{
    char stk[MAXSIZE][MAX_ARG_LEN+5];
    int top;
};
typedef struct stack STACK;
STACK s;

//Global variables
typedef enum {TRUE = 1, FALSE = 0} bool;
char main_arg[MAX_ARG][MAX_ARG_LEN];
char **env1= NULL;
char *path = NULL;
FILE *fp ;
int num_args = 0;
int offset = 0;
int interrupt_flag = 0;

//Function Prototypes

void setoffset();
void getoffset();
bool shell_redirection(char *);
bool shell_pipe(char *);
bool single_pipe(char *);
void prepare_execute(char *);
void* macro_tilde(char **);
int execute(char arg[MAX_ARG][MAX_ARG_LEN]);
int split_line(char *lin, char (*arg)[MAX_ARG][MAX_ARG_LEN]);
char* read_line();
void loop();
size_t count(const char*, char c);
void handle_SIGQUIT();
void handling_handler();
void push(char *);
void  pop();

//FUNCTION DEFINITIONS

void push (char *matchline)
{
  if (s.top == (MAXSIZE - 1))
  {
    return;
  }
  else
  {
    s.top = s.top + 1;
    strcpy(s.stk[s.top], matchline);
  }
  return;
}

/*  Function to delete an element from the stack */
void pop ()
{
  //char* str=(char *)malloc((MAX_ARG_LEN+4)*sizeof(char));
  if (s.top == - 1)
  {
    printf ("No Recommendations\n");
    //return (s.top);
  }
  else
  {
    char * x = strtok(s.stk[s.top]," ");
    //strcpy(str, s.stk[s.top]);
    x=strtok(NULL,"\n");
    printf ("\n%s\n", x);
    s.top--;
  }
  return ;
}

//Function for clear screen
// void clearScreen()
// {
//   const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
//   write(STDOUT_FILENO,CLEAR_SCREE_ANSI,12);
// }
size_t count(const char* s, char c) 
{
  size_t r = 0;
  for(; *s; s++)
    r += *s == c;
  return r;
}

void setoffset()
{
  fp = fopen(const_file, "w+");
  fprintf(fp,"%d",offset) ;
  fclose(fp); 
}
bool single_pipe(char* single_command)
{
  char cpystr[MAX_ARG_LEN];
  char* ptr[2];
  int pipe_fd[2];
  int pid;

  if(strchr(single_command, '|') != NULL)
  {
    strcpy(cpystr, single_command);
    ptr[0] = strtok(cpystr, "|");
    ptr[1] = strtok(NULL, "|");

    if(pipe(pipe_fd) == -1)
    {
      printf("%s\n", "Pipe call Error");
      return FALSE;
    }

    if((pid = fork()) == 0)
    {
      close(1);
      dup(pipe_fd[1]);
      close(pipe_fd[0]);
      if(!shell_redirection(ptr[0]))
        exit(FALSE);
      exit(TRUE);
    }
    if((pid = fork()) == 0)
    {
      close(0);
      dup(pipe_fd[0]);
      close(pipe_fd[1]);
      if(!shell_redirection(ptr[1]))
        exit(FALSE);
      exit(TRUE);
    }
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      wait(NULL);
  }
  else
  {
    if(!shell_redirection(single_command))
      return FALSE;
  }
  return TRUE;
}

bool shell_pipe(char* single_command)
{
  char cpystr[MAX_ARG_LEN];
  char* ptr[MAX_ARG];
  int pipe_fd[MAX_ARG_LEN-1][2];
  int pipe_count = 0, loop, pid;

  if(strchr(single_command, '|') != NULL)
  {
    strcpy(cpystr, single_command);
    ptr[pipe_count] = strtok(cpystr, "|");
    while(ptr[pipe_count] != NULL)
    {
      pipe_count++;
      ptr[pipe_count] = strtok(NULL, "|");    
    }

    for(loop = 0; loop < pipe_count; loop++)
    {
      if(pipe(pipe_fd[loop]) == -1)
      {
        printf("%s\n", "Pipe call Error");
          return FALSE;
      }

    }
    for(loop = 0; loop < pipe_count - 1; loop++)
    {
      if(fork() == 0)
      {
        if(loop != 0)
        {
          close(0);
          dup(pipe_fd[loop-1][0]);
          close(pipe_fd[loop-1][1]);
        }
        close(1);
        dup(pipe_fd[loop][1]);
        close(pipe_fd[loop][0]);
        if(!shell_redirection(ptr[loop]))
          exit(FALSE);
        exit(TRUE);
      }
    }
    if(fork() == 0)
    {
      close(0);
      dup(pipe_fd[pipe_count-2][0]);
      close(pipe_fd[pipe_count-2][1]);
      if(!shell_redirection(ptr[pipe_count-1]))
        exit(FALSE);
      exit(TRUE);
    }
    for(loop = 0; loop < pipe_count; loop++)
    {
      close(pipe_fd[loop][0]);
      close(pipe_fd[loop][1]);
    }
    wait(NULL);
  }
  else
  {
    if(!shell_redirection(single_command))
      return FALSE;
  }
  return TRUE;
}


bool shell_redirection(char* single_command)
{
  char *pre_red, *pro_red, *pre_cmd, *pro_cmd, *file_name;
  char cpystr[MAX_ARG_LEN];
  int red_fd, pid;

  //printf("Here");
  strcpy(cpystr, single_command);
  if((pre_red = strchr(single_command, '>')) != NULL)
  {
    pro_red = strrchr(single_command, '>');
    //printf("\n: yo3\n");
    if((pro_red-pre_red) == 1)
    {
      pre_cmd = strtok(cpystr, ">");
      pro_cmd = strtok(NULL, ">");
      file_name = strtok(pro_cmd, " ");
      if((pid = fork()) == 0)
      {
        if((red_fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1)
        {
          printf("%s\n", "Redirection error");
          return FALSE;
        }
        //printf("\n:Here yoyo:\n");
        close(1);
        dup(red_fd);
        //printf("\n:Here yoyo2:\n");
        prepare_execute(pre_cmd);
        close(red_fd);
        //printf("\n:Here yoyo3:\n");
        exit(TRUE);
      }
      waitpid(pid, NULL, 0);
    }
    else
    {
      pre_cmd = strtok(cpystr, ">");
      pro_cmd = strtok(NULL, ">");
      file_name = strtok(pro_cmd, " ");
      if((pid = fork()) == 0)
      {
        if((red_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
        {
          printf("%s\n", "Redirection error");
          return FALSE;
        }
        //printf("\n:Here yoyo4:\n");
        close(1);
        dup(red_fd);
        //printf("\n:Here yoyo5:\n");
        prepare_execute(pre_cmd);
        close(red_fd);
        //printf("\n:Here yoyo6:\n");        
        exit(TRUE);
      }
      waitpid(pid, NULL, 0);
    }
  }
  else if(strchr(single_command, '<') != NULL)
  {
    pre_cmd = strtok(cpystr, "<");
    pro_cmd = strtok(NULL, "<");
    file_name = strtok(pre_cmd, " ");
    if((pid = fork()) == 0)
    {
      if((red_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
      {
        printf("%s\n", "Redirection error");
        return FALSE;
      }
      close(1);
      dup(red_fd);
      prepare_execute(pro_cmd);
      close(red_fd);
      exit(TRUE);
    }
      waitpid(pid, NULL, 0);
  }
  else
  {
    prepare_execute(single_command);    
  }
    return TRUE;
}

void prepare_execute(char* command)
{
  char array_buffer[MAX_ARG][MAX_ARG_LEN];
  char** args;
  int idx, loop;

  idx = split_line(command, &array_buffer);
  // args = (char**)malloc(sizeof(char*)*(idx+1));
  // for(loop = 0; loop<idx; loop++)
  //   args[loop] = array_buffer[loop];
  // args[idx] = NULL;
  execute(array_buffer);
  //free(args);
}

void* macro_tilde(char** string)
{
  char* env_home;
  char tmpstr[MAX_ARG_LEN];

  env_home = getenv("HOME");
  strcpy(tmpstr, env_home);
  strcat(tmpstr, *(string)+1);
  strcpy(*string, tmpstr);
}

//MAIN EXECUTION FUNCTION
int execute(char arg[MAX_ARG][MAX_ARG_LEN])
{
  //printf("\n yo yo %sabc, %d", arg[0], (int)strlen(arg[0]));
  if(strcmp(arg[0], "clear") == 0)
  {
      //printf("\nyoyo");
    if(arg[1][0] != '\0')
      printf("No Command '%s %s' found\n",arg[0], arg[1] ) ;
    else
      system("clear");
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
    //printf("\n%s, %d", arg[1], (int)strlen(arg[1]));
    char temp_path[MAX_ARG_LEN];
    char* ptr = arg[1];
    strcpy(temp_path, arg[1]);
    if(temp_path[0] == '~')
      macro_tilde(&ptr);
    if(chdir(ptr) == -1)
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
    bool isnotvalid = FALSE;
    char *historyLine = NULL;
    size_t len; 
    ssize_t read;
    if(num_args > 2)
    {
      printf("History: too many arguments!\n");
      return 0;
    }

    if(arg[1][0] != '\0')
    {
      int i = 0;
      for( ;arg[1][i] !='\0';i++)
      {
        if(arg[1][i] <'0' || arg[1][i] > '9')
        {
          printf("No Command '%s %s' found\n", arg[0], arg[1] );
          isnotvalid = TRUE;
          break;
        }
        else
          num = num*10 + (arg[1][i] -'0');
      }
      if(isnotvalid) return -1;

      fp = fopen(history_file,"r");
      //int flag = 0;
      do
      {
        while ((read = getline(&historyLine, &len, fp)) != -1) 
        {
          off = 0;
          for(int s = 0; s< strlen(historyLine); s++)
          {
            if(historyLine[s]!= ' ')
            {
              off = off*10 + (historyLine[s] -'0');
              //printf("\n: %d : \n", off);
              //printf(" yoyo");
              //flag = 1;
            }
            else break;
          }
          //printf(" %d : ", off);
          if(offset-off < num)
          {
            printf("%s", historyLine);
          }
          //off++;
        }
        //fscanf(fp,"%d %s", &off,command);
        
      }
      while( off < offset && !feof(fp));
    }
    else
    {
      fp = fopen(history_file,"r");
       
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
    char c[MAX_ARG_LEN];
    strcpy(c,"./");
    strcat(c , arg[0]);
    int len_arg = strlen(arg[1]);
    char **args;
    args = (char **)malloc(MAX_ARG*sizeof(char*));
    for(int i = 0; i<num_args; i++)
    {
       args[i] = arg[i];
    }
    if(fork() == 0)
    {
      if(arg[1][len_arg-1] == '&')
        arg[1][len_arg-1] = '\0' ;
      //printf("\nCame here as : %s, %s, %s\n", arg[0], arg[1], arg[2]);
      ret = execvp(*args, args);
      if(ret == -1)
      {
        printf("Unknown command : %s\n", arg[0]);
        // ret = execl(c,arg[0],arg[1],(char *)NULL);
        // if(ret == -1)
        //  perror("The following error(s) occured in the current directory.");
        // else
        // {
        //   getcwd(c,sizeof(c));
        //   printf("%s>",c);
        // } 
      }
      else
      {
        getcwd(c,sizeof(c));
        printf("%s>",c);
      } 
    }
    else
    {
      if(args[1][len_arg-1] != '&')
        wait(&child);
    }
  }
    
  return 0;
}

//Breaking Up arguments
int split_line(char *lin, char (*arg)[MAX_ARG][MAX_ARG_LEN])
{

    char* ptr;
    int idx = 0;

    ptr = strtok(lin, " ");
    while(ptr){
        strncpy((*arg)[idx++], ptr, MAX_ARG_LEN);
        ptr = strtok(NULL," ");
        //printf("\n : %s :\n", (*arg)[idx -1]);
    }

    return idx;
    
}
//   int i, j, k1;
//   char temp = ' ';
//   for (i = 0, j = 0, k1 = 0; i < sizeof(lin)/sizeof(char); i++ )
//   {
//     if(lin[i]=='\n')continue;

//     if(lin[i] == ' ')
//     {
//       if(temp == ' ')continue;
//       else if(temp != ' ')
//       {
//         j = 1;
//         arg[0][k1]='\0';
//         strcpy(arg[1], (lin+i+1));
//         break;
//       }
//     }
//     else if(!j)
//     {
//       temp = lin[i];
//       arg[j][k1++] = temp;
//     }
//   }
//   if(!j) arg[0][k1] = arg[1][0] = '\0';
// }

//Basic read-line from user
char* read_line()
{
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us
  ssize_t length;
  fflush(stdin);
  fflush(stdin);
  fflush(stdin);
  //do 
  //{
    length=getline(&line, &bufsize, stdin);
    //    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
  //}while (length == -1 && errno == EINTR);
  fflush(stdin);
  fflush(stdin);
  return line;
}

//Main loop function
void loop()
{
  char *line;
  char temp_line[100];
  //char arg[MAX_ARG][MAX_ARG_LEN];
  int status;
  size_t size = 100*sizeof(char);
  setoffset();

  while(1) 
  {
    //Prompt Display
    //if(!interrupt_flag){
    offset++;
    path = getcwd(path,size);
    printf("%s > ",path);
    
    //Read Line
    line = read_line();
    line[strlen(line)-1] = '\0';
    

    //Write line ot file for easy history recall
    fp = fopen(history_file, "a+");
    fprintf(fp,"%d %s\n",offset, line) ;
    fclose(fp) ;
    setoffset();

    
    //Parse Line and get argument array
    //arg [0][0] = arg[1][0] = '\0';
    num_args = split_line(line, &main_arg);
    //printf("Arg 1 : %s \nArg 2 : %s", arg[0], arg[1]);
    
    //Execute the user commands
    //status = execute(main_arg);
    if(strchr(line, '|') == NULL)
    {  
      shell_redirection(line);
    }
    else if(count(line,'|')==1)
    {
      single_pipe(line);
    }
    else if(count(line,'|')>1)
    {
      shell_pipe(line);
    }//}
    //else handling_handler();

    
    //free(line);
     
  }
}

//Getting and Setting offset for multiple executions
void getoffset()
{
  fp = fopen(const_file, "a");
  fclose(fp);

  fp = fopen(const_file, "r+");
  fscanf(fp,"%d",&offset) ;
  fclose(fp); 
}

void handle_SIGQUIT()
{
  printf("\nEnter the character or string(without spaces) which you want to search for \n");
  char *c=NULL;//=(char *)malloc(MAX_ARG_LEN*sizeof(char)) ;
  char *historyLine = NULL;
  size_t len = 0, buf =0; //MAX_ARG_LEN; 
  ssize_t read;
  int i= 0 ;
  fflush(stdin);
  fflush(stdin);
  fflush(stdin);
  getline(&c, &buf, stdin);
  fflush(stdin);
  fflush(stdin);
  fflush(stdin);
  //c[strlen(c)-1]='\0';
  //printf("\n yo %s \n", c);
  int off = 0;
  fp = fopen(history_file,"r");
      //int flag = 0;
  do
  {
    while ((read = getline(&historyLine, &len, fp)) != -1) 
    {
      //printf("\n yoyo %s : %s \n", c, historyLine);
      if(strstr(historyLine, c) != NULL)
      {
        push(historyLine);
      }
    }
  }
  while(!feof(fp));
  fclose(fp);
  i = 0;
  while(s.top!=-1 && i<10)
  {
    pop();
    i++;
  }

}

void handling_handler()
{
    
}

int main(int argc, char **argv, char **envp)
{
  s.top = -1;
  struct sigaction handler;
  handler.sa_handler = handle_SIGQUIT;
  sigaction(SIGQUIT, &handler, NULL);
  //signal(SIGQUIT,sigquit_overloaded);
  env1=envp;
  

  getoffset();
  // Run command loop.
  loop();

  
  return 0;

}
