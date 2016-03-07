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
#include <sys/msg.h>
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
const char history_file[] = "/home/mankeyboy/Git_maintained/OS_Lab/Assignment_3/History_file.txt";
const char const_file[] = "/home/mankeyboy/Git_maintained/OS_Lab/Assignment_3/Const_file.txt";
//const char history_file[] = "/home/raagapriya/Documents/os_lab/assignment3/History_file.txt";
//const char const_file[] = "/home/raagapriya/Documents/os_lab/assignment3/Const_file.txt";

#define MSGSZ   4090


//Main message queue type
typedef struct msgbuf 
{
  long    mtype;
  struct msg
  {
    char    mtext[MSGSZ];
    int    pid;
  }message;
}message_buf;

//Global variables
typedef enum {TRUE = 1, FALSE = 0} bool;
char main_arg[MAX_ARG][MAX_ARG_LEN];
char main_line[MAX_ARG_LEN];
char **env1= NULL;
char *path = NULL;
FILE *fp ;
int num_args = 0;
int offset = 0;
int flag_send = 0;

int msqid;
int msgflg = IPC_CREAT | 0666;//access flags
key_t key;
message_buf sbuf,rbuf;
size_t buf_length;
char  glob_lin[255];
int pid;





//strcat(sbuf.message.mtext,"Terminal  ");

char p[10];
/*memset(p,'_',10);
sprintf(p, "%d", pid);
strcat(sbuf.message.mtext,p);
strcat(sbuf.message.mtext,"\n");
*/

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
void send_msg();

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

void send_msg()
{

  if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) 
  {
    perror("msgsnd");
    exit(1);
  }
  
  else 
    printf("Message Sent to \"%ld\"  \n", sbuf.mtype);
  
  memset(sbuf.message.mtext,0,sizeof(sbuf.message.mtext));
  //strcat(sbuf.message.mtext,p,10);
  //strcat(sbuf.message.mtext,"\n");
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

  //printf(" Main cmd: %s ||", single_command);
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
  //char** args;
  int idx, loop;
  //printf("Main cmd: %s|", command);
  idx = split_line(command, &array_buffer);
  //printf("Arg 1 : %s \nArg 2 : %s", array_buffer[0], array_buffer[1]);
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
    {
        
        strcat(sbuf.message.mtext,"No Command ");
        strcat(sbuf.message.mtext,glob_lin);
        strcat(sbuf.message.mtext," found \n");
        printf("No Command '%s %s' found\n",arg[0], arg[1] ) ;
    }

    else
    {
      strcat(sbuf.message.mtext,"clear\n");
      system("clear");
    }

    if(flag_send == 1)
         send_msg();
 
  }
  
  else if(strcmp(arg[0], "env") == 0)
  {
    if(arg[1][0] != '\0')
    {

      strcat(sbuf.message.mtext,"No Command ");
      strcat(sbuf.message.mtext,glob_lin);
      strcat(sbuf.message.mtext," found \n");
      

      printf("No Command '%s %s' found\n",arg[0], arg[1] ) ;
    }
    else
    {
      char **env;
      strcat(sbuf.message.mtext,glob_lin);
      strcat(sbuf.message.mtext,"\n");
      for(env=env1;*env !=0; env++)
      {
        char *thisEnv = *env;
        strcat(sbuf.message.mtext,thisEnv);
        strcat(sbuf.message.mtext,"\n");
        printf("%s\n", thisEnv);
      }
    }
    
    if(flag_send == 1)
         send_msg();

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
      strcat(sbuf.message.mtext,glob_lin);
      strcat(sbuf.message.mtext,"\n");
      getcwd(path, sizeof(path));
    }
    
    if(flag_send == 1)
        send_msg();
  }
  
  else if(strcmp(arg[0], "pwd")==0)
  {
    getcwd(path, sizeof(path));
    strcat(sbuf.message.mtext,glob_lin);
    strcat(sbuf.message.mtext,"\n");
    strcat(sbuf.message.mtext,path);
    printf("%s\n",path);
    if(flag_send == 1)
        send_msg();
  }
  
  else if(strcmp(arg[0], "mkdir")==0)
  {
    int ret;
    ret = mkdir(arg[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    strcat(sbuf.message.mtext,glob_lin);
    strcat(sbuf.message.mtext,"\n");
    if(flag_send == 1)send_msg();
    if(ret == -1)
      perror("The following error(s) occured.");
  }

  else if(strcmp(arg[0], "rmdir")== 0)
  {
    int ret;
    ret = rmdir(arg[1]);
    strcat(sbuf.message.mtext,glob_lin);
    strcat(sbuf.message.mtext,"\n");
    if(flag_send)send_msg();
    if(ret == -1)
      perror("The following error(s) occured.");
  }

  else if(strcmp(arg[0], "ls") == 0) 
  {

    strcat(sbuf.message.mtext,glob_lin);
    strcat(sbuf.message.mtext,"\n");
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
        int i=0,m;
        long z;
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
        
        strcat(sbuf.message.mtext,(S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
        strcat(sbuf.message.mtext,(fileStat.st_mode & S_IXOTH) ? "x" : "-");
        printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
        
        
         
        printf("  %d",(int)fileStat.st_nlink);
        
        m = (int)fileStat.st_nlink;
        memset(p,'_',10);
        sprintf(p, "%d", m);
        strcat(sbuf.message.mtext,p);
        strcat(sbuf.message.mtext,"  ");
        printf("  %s",pwuser->pw_name);
        strcat(sbuf.message.mtext,pwuser->pw_name);
        strcat(sbuf.message.mtext,"  ");
        printf("  %s",grpname->gr_name);
        strcat(sbuf.message.mtext,grpname->gr_name);
        strcat(sbuf.message.mtext,"  ");
        printf("  %ld",fileStat.st_size);
         z = (long)fileStat.st_size;
        memset(p,'_',10);
        sprintf(p, "%d", (int)z);
        strcat(sbuf.message.mtext,p);
        strcat(sbuf.message.mtext,"  ");
        printf("   %s",ctime(&fileStat.st_atim.tv_sec));
         strcat(sbuf.message.mtext,ctime(&fileStat.st_atim.tv_sec));
        strcat(sbuf.message.mtext,"  ");
        printf("   %s\n",sd->d_name);
        strcat(sbuf.message.mtext,sd->d_name);
        strcat(sbuf.message.mtext,"\n");
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
        {
              printf ("%s\t", ent->d_name);
              strcat(sbuf.message.mtext, ent->d_name);
              strcat(sbuf.message.mtext,"\t");
        }
        closedir (dir);
      } 
      else perror ("");
      strcat(sbuf.message.mtext,"\n");
      printf("\n");
      
    }
      
    else 
    {
         strcat(sbuf.message.mtext,"No such command found");
         strcat(sbuf.message.mtext,"\n");
         printf("No such command found\n") ; 
    } 
   send_msg();
  }

  else if(strcmp(arg[0],"history") == 0)
  {

    strcat(sbuf.message.mtext,glob_lin);
    strcat(sbuf.message.mtext,"\n");
    int num = 0, off;
    bool isnotvalid = FALSE;
    char *historyLine = NULL;
    size_t len; 
    ssize_t read;
    if(num_args > 2)
    {
      strcat(sbuf.message.mtext,"History: too many arguments!");
      strcat(sbuf.message.mtext,"\n");
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
          strcat(sbuf.message.mtext,"No such command found");
          strcat(sbuf.message.mtext,"\n");
          printf("No such command found\n" ) ;
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
          int s;
          for( s = 0; s< strlen(historyLine); s++)
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
            strcat(sbuf.message.mtext,historyLine);
            
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
          {
              strcat(sbuf.message.mtext,historyLine);
              printf("%s", historyLine);
          }
            
    }
    if(flag_send == 1)send_msg();
  }
    
  else if(strcmp(arg[0], "exit") == 0)
  {
    strcat(sbuf.message.mtext,glob_lin);
    if(flag_send == 1)send_msg();
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
    int i;
    for( i = 0; i<num_args; i++)
    {
       args[i] = arg[i];
    }
    int pipefd[2];
    pipe(pipefd);

    if((child = fork()) == 0)
    {
      close(pipefd[0]);    // close reading end in the child

      dup2(pipefd[1], 1);  // send stdout to the pipe
      dup2(pipefd[1], 2);  // send stderr to the pipe

      if(arg[1][len_arg-1] == '&')
        arg[1][len_arg-1] = '\0' ;
      //printf("\nCame here as : %s, %s, %s\n", arg[0], arg[1], arg[2]);
      ret = execvp(*args, args);
      if(ret == -1)
      {
        //printf("Unknown command : %s\n", arg[0]);
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
      char buffer[1024];

      close(pipefd[1]);  // close the write end of the pipe in the parent

      while (read(pipefd[0], buffer, sizeof(buffer)) != 0)
      {
      }
      strcat(sbuf.message.mtext,glob_lin);
      strcat(sbuf.message.mtext,"\n");
      strcat(sbuf.message.mtext, buffer);
      if(flag_send == 1)send_msg();
      
      if(main_line[strlen(main_line)-1] == '&')
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
    char abc[MAX_ARG_LEN];
    strcpy(abc, lin);
    ptr = strtok(abc, " ");
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
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  strcpy(main_line, line);
  return line;
}


//Main loop function
void loop()
{

   printf("debug3\n");
  char *line;
  char temp_line[100];
  //char arg[MAX_ARG][MAX_ARG_LEN];
  int status,flag_1 = 0;
  size_t size = 100*sizeof(char);
  setoffset();
  message_buf coupling_buf;
  coupling_buf.message.pid = sbuf.message.pid;
  coupling_buf.mtype = sbuf.mtype;
  buf_length = sizeof(struct msg);
  pid_t pid_receiver;
  if((pid_receiver = fork())!= 0)
  {
    while(1) 
    {
      //Prompt Display
      offset++;
      path = getcwd(path,size);
      printf("%s > ",path);
      
      line = read_line();
      line[strlen(line)-1] = '\0';
      strcpy(glob_lin, line);
      

      //Write line ot file for easy history recall
      fp = fopen(history_file, "a+");
      fprintf(fp,"%d %s\n",offset, line) ;
      fclose(fp) ;
      setoffset();

      if(strcmp(glob_lin,"couple") == 0)
      {
         strcpy(coupling_buf.message.mtext,"couple");
         printf("\n %d, %d, %s, %ld, %d", msqid, coupling_buf.message.pid, coupling_buf.message.mtext, coupling_buf.mtype, (int)sizeof(struct msg));
         if (msgsnd(msqid, &coupling_buf, sizeof(coupling_buf.message), IPC_NOWAIT) < 0) 
            {
                perror("msgsnd1");
                exit(1);
            }

         else 
            {
                flag_send = 1;
                printf("Message Sent to \"%ld\"  \n", coupling_buf.mtype);
            }
         continue;

      }

      if(strcmp(glob_lin,"uncouple") == 0)
      {
         strcpy(coupling_buf.message.mtext,"uncouple");

         if (msgsnd(msqid, &coupling_buf, buf_length, IPC_NOWAIT) < 0) 
            {
                perror("msgsnd2");
                exit(1);
            }

         else 
            { 
                flag_send = 0;
                printf("Message Sent to \"%ld\"  \n", coupling_buf.mtype);
            }
         continue;
       
      }
      


      
      //Parse Line and get argument array
      //arg [0][0] = arg[1][0] = '\0';
      num_args = split_line(line, &main_arg);
      //printf("Arg 1 : %s \nArg 2 : %s", main_arg[0], main_arg[1]);
      
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
      }

      
      //free(line);
       
    }
  }
  else
  {
     while(1)
    {
      if (msgrcv(msqid, &rbuf, sizeof(struct msg), pid, IPC_NOWAIT) < 0  ) 
      {
          
          if(errno == ENOMSG)
            {}
          else
          {
            perror("msgrcv");
            exit(1);
          }
      }
      else{
        printf("%s\n", rbuf.message.mtext);
        path = getcwd(path,size);
        printf("%s > ",path);
      }
      
    }
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


int main(int argc, char **argv, char **envp)
{

  key = 1234;
  (sbuf.mtype) = 1;


  pid = getpid();
  sbuf.message.pid = pid;
  
  printf("My pid = %d\n",pid);
  memset(sbuf.message.mtext,0,sizeof(sbuf.message.mtext));


  if ((msqid = msgget(key, msgflg )) < 0) 
   {
        perror("msgget");
        exit(1);      
   }
  
  else 
   printf("message queue created : %d\n",msqid);
  
  env1=envp;
  
   printf("debug1\n");
  getoffset();
   printf("debug2\n");
  // Run command loop.
  loop();
  
  return 0;

}
