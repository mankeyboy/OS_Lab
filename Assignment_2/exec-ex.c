/* ----------------------------------------------------------------- */
/* PROGRAM  shell.c                                                  */
/*    This program reads in an input line, parses the input line     */
/* into tokens, and use execvp() to execute the command.             */
/* ----------------------------------------------------------------- */

#include  <stdio.h>
#include  <sys/types.h>

/* ----------------------------------------------------------------- */
/* FUNCTION  parse:                                                  */
/*    This function takes an input line and parse it into tokens.    */
/* It first replaces all white spaces with zeros until it hits a     */
/* non-white space character which indicates the beginning of an     */
/* argument.  It saves the address to argv[], and then skips all     */
/* non-white spaces which constitute the argument.                   */
/* ----------------------------------------------------------------- */

void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
}

/* ----------------------------------------------------------------- */
/* FUNCTION execute:                                                 */
/*    This function receives a commend line argument list with the   */
/* first one being a file name followed by its arguments.  Then,     */
/* this function forks a child process to execute the command using  */
/* system call execvp().                                             */
/* ----------------------------------------------------------------- */
     
void  execute(char **argv)
{
     pid_t  pid;
     int    status;
     
     if ((pid = fork()) < 0) {     /* fork a child process           */
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {          /* for the child process:         */
          if (execvp(*argv, argv) < 0) {     /* execute the command  */
               printf("*** ERROR: exec failed\n");
               exit(1);
          }
     }
     else {                                  /* for the parent:      */
          while (wait(&status) != pid)       /* wait for completion  */
               ;
     }
}

/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */
     
void  main(void)
{
     char  *line=NULL;             /* the input line                 */
     char  *argv[64];              /* the command line argument      */
     ssize_t buf = 0;
     while (1) {                   /* repeat until done ....         */
          printf("Shell -> ");     /*   display a prompt             */
          getline(&line, &buf, stdin);              /*   read in the command line     */
          printf("\n");
          parse(line, argv);       /*   parse the line               */
          if (strcmp(argv[0], "exit") == 0)  /* is it an "exit"?     */
               exit(0);            /*   exit if it is                */
          execute(argv);           /* otherwise, execute the command */
     }
}

                