/*
*   Program Name  : smsh (Small Shell)
*   All reserved by OwlFox
*   Property    : This shell is based on FreeBSD
*   Facilities  :
*       - foreground and background execution (&)
*       - multiple commands separated by semicolons
*       - history command (history, !)
*       - shell redirection (>, >>, <)
*       - shell pipe (ls –la | more)
*       - Multiple pipe (ls  | grep “^d” | more)
*       - cd command
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#define STRING_BUFFER_SIZE  255
#define ARRAY_BUFFER_SIZE   16
#define HISTORY_BUFFER_SIZE 500

typedef enum {TRUE = 1, FALSE = 0} bool;

int parse_command(char*, char(*)[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE]);
void command_switch(int, char**);
void macro_tilde(char**);
int multiple_command(char*, char(*)[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE]);
//void bg_processing(int, char**);
bool bg_processing(char*);
bool single_pipe(char*);
bool shell_pipe(char*);
bool shell_redirection(char*);
void prepare_execute(char*);

//Shell Built-in Function Prototype
void cmd_cd(char**);
void cmd_exit(void);
void cmd_pwd(void);
void cmd_history(int, char**);
int cmd_run_history(char**, char(*)[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE], int*);

//History Global Variable
int history_count;
char history_data[HISTORY_BUFFER_SIZE][STRING_BUFFER_SIZE];

int main(void){
    char input_buffer[STRING_BUFFER_SIZE];
//char array_buffer[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE];
    char multi_command[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE];
// char** args;
    int multi_cnt;
    int idx;
    int loop1;
    int loop2;

    history_count = 0;

// Shell Loop
    while(1){
// Shell Curser
        fflush(stdin);
        printf("%s","smsh $: ");
// User Input
        fgets(input_buffer, STRING_BUFFER_SIZE, stdin);
        input_buffer[strlen(input_buffer)-1] = '\0';
// If there are not any input, loop again
        if(strlen(input_buffer)!=0){
// Parsing input stinrg to multi command string
            multi_cnt = multiple_command(input_buffer, &multi_command);
// If a error is exist, loop again
            if(multi_cnt == -1)
                continue;
// Execute each commands
            for(loop1 = 0; loop1<multi_cnt; loop1++){
                bg_processing(multi_command[loop1]);
// // parsing
// idx = parse_command(multi_command[loop1], &array_buffer);
// args = (char**)malloc(sizeof(char*)*(idx+1));
// for(loop2 = 0; loop2<idx; loop2++)
//  args[loop2] = array_buffer[loop2];
// args[idx] = NULL;
// if((strlen(args[idx-1])==1)&&args[idx-1][0] == '&'){
//  bg_processing(idx, args);
// }
// else
//  command_switch(idx, args);
// free(args);
            }
        }
    }
}

int parse_command(char* full_command, char (*args)[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE]){
    char* ptr;
    int idx = 0;

    ptr = strtok(full_command, " ");
    while(ptr){
        strncpy((*args)[idx++], ptr, STRING_BUFFER_SIZE);
        ptr = strtok(NULL," ");
    }

    return idx;
}

void command_switch(int argc,char** args){
    pid_t pid;
    int status;
    char* extend_path;

//If there are no input, finish!
    if(argc == 0)
        return;
//Using Built-in Function
    if(0){}
        else if(!strcmp(args[0], "cd"))
            cmd_cd(args);
        else if(!strcmp(args[0], "exit"))
            cmd_exit();
        else if(!strcmp(args[0], "pwd"))
            cmd_pwd();
        else if(!strcmp(args[0], "history"))
            cmd_history(argc, args);
//  else if(args[0][0] == '!')
//      cmd_run_history(argc, args);
        else{
            extend_path = (char*)malloc(sizeof(char)*(6+strlen(args[0])));
            if((pid = fork()) < 0){
                printf("fork error\n");
                free(extend_path);
                exit(1);
            }else if(pid == 0){
                if(execvp(args[0], args) == -1){
                    printf("Unknown command : %s\n", args[0]);
                    exit(0);
                }
            }else{
                waitpid(pid, NULL, 0);
                free(extend_path);
            }
        }
    }

    void macro_tilde(char** string){
        char* env_home;
        char tmpstr[STRING_BUFFER_SIZE];

        env_home = getenv("HOME");
        strcpy(tmpstr, env_home);
        strcat(tmpstr, *(string)+1);
        strcpy(*string, tmpstr);
    }

    int multiple_command(char* full_command, char (*commands)[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE]){
        char* ptr;
        int idx = 0,loop;

        ptr = strtok(full_command, ";");
        while(ptr){
            if(ptr[0] == '!'){
                if((cmd_run_history(&ptr, commands, &idx) == -1))
                    return -1;
            }else{
                strncpy((*commands)[idx++], ptr, STRING_BUFFER_SIZE);   
            }
            ptr = strtok(NULL,";");
        }
        strcpy(history_data[history_count%HISTORY_BUFFER_SIZE], (*commands)[0]);
        for(loop = 1; loop < idx; loop++){
            strcat(history_data[history_count%HISTORY_BUFFER_SIZE], ";");
            strcat(history_data[history_count%HISTORY_BUFFER_SIZE], (*commands)[loop]);
        }
        history_count++;
        return idx;
    }
    bool bg_processing(char* single_command){
        char cpystr[STRING_BUFFER_SIZE];
        char* ptr[ARRAY_BUFFER_SIZE];
        int bg_count = 0, loop, pid;
        int first_cmd_len = strlen(single_command);

        for(loop = 0; loop < first_cmd_len; loop++){
            if(single_command[loop] == ' ');
            else if(single_command[loop] == '&'){
                printf("syntax error near unexpected token '&'\n");
                return FALSE;
            }
            else
                break;
        }

        if(strchr(single_command, '&') != NULL){
            strcpy(cpystr, single_command);
            strcat(cpystr, " ");

            ptr[bg_count] = strtok(cpystr, "&");
            while(ptr[bg_count] != NULL){
                bg_count++;
                ptr[bg_count] = strtok(NULL, "&");
            }
            for(loop = 0;loop < bg_count-1;loop++){
                if((pid = fork()) == 0){
                    if(!shell_pipe(ptr[loop]))
                        exit(FALSE);
                    exit(TRUE);
                }
                printf("BG [%d]\n", pid);
            }
            if(!shell_pipe(ptr[bg_count-1]))
                return FALSE;
        }else{
            if(!shell_pipe(single_command))
                return FALSE;
        }
        return TRUE;
    }
// void bg_processing(int argc, char** args){
//  printf("%s\n", "Background Processing...");
//  if(fork() == 0){
//      command_switch(argc, args);
//      exit(1);
//  }
// }

    bool single_pipe(char* single_command){
        char cpystr[STRING_BUFFER_SIZE];
        char* ptr[2];
        int pipe_fd[2];
        int pid;

        if(strchr(single_command, '|') != NULL){
            strcpy(cpystr, single_command);
            ptr[0] = strtok(cpystr, "|");
            ptr[1] = strtok(NULL, "|");

            if(pipe(pipe_fd) == -1){
                printf("%s\n", "Pipe call Error");
                return FALSE;
            }

            if((pid = fork()) == 0){
                close(1);
                dup(pipe_fd[1]);
                close(pipe_fd[0]);
                if(!shell_redirection(ptr[0]))
                    exit(FALSE);
                exit(TRUE);
            }
            if((pid = fork()) == 0){
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
        }else{
            if(!shell_redirection(single_command))
                return FALSE;
        }
        return TRUE;
    }

    bool shell_pipe(char* single_command){
        char cpystr[STRING_BUFFER_SIZE];
        char* ptr[ARRAY_BUFFER_SIZE];
        int pipe_fd[ARRAY_BUFFER_SIZE-1][2];
        int pipe_count = 0, loop, pid;

        if(strchr(single_command, '|') != NULL){
            strcpy(cpystr, single_command);
            ptr[pipe_count] = strtok(cpystr, "|");
            while(ptr[pipe_count] != NULL){
                pipe_count++;
                ptr[pipe_count] = strtok(NULL, "|");    
            }

            for(loop = 0; loop < pipe_count; loop++){
                if(pipe(pipe_fd[loop]) == -1){
                    printf("%s\n", "Pipe call Error");
                    return FALSE;
                }

            }
            for(loop = 0; loop < pipe_count - 1; loop++){
                if(fork() == 0){
                    if(loop != 0){
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
            if(fork() == 0){
                close(0);
                dup(pipe_fd[pipe_count-2][0]);
                close(pipe_fd[pipe_count-2][1]);
                if(!shell_redirection(ptr[pipe_count-1]))
                    exit(FALSE);
                exit(TRUE);
            }
            for(loop = 0; loop < pipe_count; loop++){
                close(pipe_fd[loop][0]);
                close(pipe_fd[loop][1]);
            }
            wait(NULL);
        }else{
            if(!shell_redirection(single_command))
                return FALSE;
        }
        return TRUE;

    }

    bool shell_redirection(char* single_command){
        char *pre_red, *pro_red, *pre_cmd, *pro_cmd, *file_name;
        char cpystr[STRING_BUFFER_SIZE];
        int red_fd, pid;

        strcpy(cpystr, single_command);
        if((pre_red = strchr(single_command, '>')) != NULL){
            pro_red = strrchr(single_command, '>');
            if((pro_red-pre_red) == 1){
                pre_cmd = strtok(cpystr, ">");
                pro_cmd = strtok(NULL, ">");
                file_name = strtok(pro_cmd, " ");
                if((pid = fork()) == 0){
                    if((red_fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1){
                        printf("%s\n", "Redirection error");
                        return FALSE;
                    }
                    close(1);
                    dup(red_fd);
                    prepare_execute(pre_cmd);
                    close(red_fd);
                    exit(TRUE);
                }
                waitpid(pid, NULL, 0);
            }else{
                pre_cmd = strtok(cpystr, ">");
                pro_cmd = strtok(NULL, ">");
                file_name = strtok(pro_cmd, " ");
                if((pid = fork()) == 0){
                    if((red_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1){
                        printf("%s\n", "Redirection error");
                        return FALSE;
                    }
                    close(1);
                    dup(red_fd);
                    prepare_execute(pre_cmd);
                    close(red_fd);
                    exit(TRUE);
                }
                waitpid(pid, NULL, 0);
            }
        }else if(strchr(single_command, '<') != NULL){
            pre_cmd = strtok(cpystr, "<");
            pro_cmd = strtok(NULL, "<");
            file_name = strtok(pre_cmd, " ");
            if((pid = fork()) == 0){
                if((red_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1){
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
        }else{
            prepare_execute(single_command);    
        }
        return TRUE;
    }
    void prepare_execute(char* command){
        char array_buffer[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE];
        char** args;
        int idx, loop;


        idx = parse_command(command, &array_buffer);
        args = (char**)malloc(sizeof(char*)*(idx+1));
        for(loop = 0; loop<idx; loop++)
            args[loop] = array_buffer[loop];
        args[idx] = NULL;
        command_switch(idx, args);
        free(args);
    }

//Shell Built-in Function
    void cmd_cd(char** args){
        char path[STRING_BUFFER_SIZE];
        char* ptr = path;

        strcpy(path, args[1]);
        if(path[0] == '~'){
            macro_tilde(&ptr);
        }
        if(chdir(ptr) == -1)
            printf("cd: %s: No such file or directory\n", ptr);
    }
    void cmd_exit(void){
        printf("exit\n");
        exit(0);    
    }
    void cmd_pwd(void){
        char cwd[STRING_BUFFER_SIZE];
        getcwd(cwd, STRING_BUFFER_SIZE);
        printf("%s\n", cwd);
    }
    void cmd_history(int argc, char** args){
        int number, loop;
        char command[STRING_BUFFER_SIZE];

        if(argc > 2){
            printf("history: too many arguments!\n");
            return;
        }
        if(argc != 1){
            if((number = atoi(args[1])) == 0)
                printf("history: %s: numeric argument is required\n", args[1]);
            if(number > HISTORY_BUFFER_SIZE)
                number = HISTORY_BUFFER_SIZE;
        }else
        number = HISTORY_BUFFER_SIZE;
        if(number > history_count)
            number = history_count;
        for(loop = 0; loop < number; loop++){
            printf("%d\t%s\n",history_count-number+loop+1,history_data[(history_count-number+loop)%HISTORY_BUFFER_SIZE]);
        }
    }
    int cmd_run_history(char** cmd, char (*commands)[ARRAY_BUFFER_SIZE][STRING_BUFFER_SIZE], int* count){
        char* ptr;
        char replaced_cmd[STRING_BUFFER_SIZE];
        char* tokenized;
        int number, idx;

        ptr = *cmd;
        idx = *count;
        tokenized = strtok(ptr, " ");
        if((number = atoi(&(tokenized[1]))) == 0){
            printf("%s: event not found\n", tokenized);
            return -1;
        }
        if(((number > history_count)||(number < 1)||(number < history_count-HISTORY_BUFFER_SIZE+1))){
            printf("%s: event not found\n", tokenized);
            return -1;
        }
        strcpy(replaced_cmd, history_data[(number-1)%HISTORY_BUFFER_SIZE]);

        while((tokenized = strtok(NULL, " "))){
            sprintf(tokenized, " %s", tokenized);
            strcat(replaced_cmd, tokenized);
        }

        ptr = strtok(replaced_cmd,";");
        strncpy((*commands)[idx++], ptr, STRING_BUFFER_SIZE);
        while((ptr = strtok(NULL,";")))
        strncpy((*commands)[idx++], ptr, STRING_BUFFER_SIZE);
        *count = idx;

        return 0;
    }