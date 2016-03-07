#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
int interrupt = 0 ;
char blah[100];

#define MAXSEGS 10
#define MAXARGS 10
#define MAXBUFF 100

void parsepipe(char *inp){
	int i, j, f, segno, argno,	k, flag, cpid, status;
	f = 0;
	for(i = 0 ; inp[i] != '\0' ; i++){
		if(inp[i] == '|'){
			f = 1;
			break;
		}
	}
	if(f == 0)
		return;

	char ***myargs;
	myargs = (char***) malloc(MAXSEGS * sizeof(char**));
	for(i = 0 ; i < MAXSEGS ; i++){
		myargs[i] = (char**) malloc(MAXARGS * sizeof(char*));
		for(j = 0 ; j < MAXARGS ; j++){
			myargs[i][j] = (char*) malloc(MAXBUFF * sizeof(char));
		}
	}
	segno = 0;
	argno = 0;
	k = 0;
	flag = 0;
	for(i = 0 ; inp[i] != '\0' ; i++){
		if(inp[i] == '|'){
			myargs[segno][argno][k] = '\0';
			k = 0;
			if(flag != 0){
				argno++;
			}
			myargs[segno][argno] = NULL;
			segno++;
			argno = 0;
			flag = 0;
		}
		else if(inp[i] == ' '){
			if(flag != 0){
				myargs[segno][argno][k] = '\0';
				argno++;
				k = 0;
				flag = 0;
			}
		}
		else if(inp[i] == '\n'){
			myargs[segno][argno][k] = '\0';
			k = 0;
			if(flag != 0)
				argno++;
			myargs[segno][argno] = NULL;
			argno = 0;
			k = 0;
			segno++;
			flag = 0;
		}
		else{
			if(flag == 0){
				flag = 1;
				k = 0;
			}
			myargs[segno][argno][k] = inp[i];
			k++;
		}
	}
	printf("%d\n",segno);
	for(i=0;i<segno;i++){
		printf("%s\n",myargs[i][0] );
	}
	int pipes[segno-1][2];
 	for(i= 0 ; i < segno-1 ; i++)
 		pipe(pipes[i]);

	for(i=0;i<segno;i++){
    if(i==0){
        pid_t ch1;
        ch1=fork();
        if(ch1==0){
          dup2(pipes[0][1],1);
          printf("%s\n",myargs[0][0] );
          execvp(myargs[0][0],myargs[0]);
          printf("exec error1\n");
        }
        else{
          waitpid(ch1,&status,0);
        }
    }
    else if(i<segno-1){
        pid_t ch1;
        ch1=fork();
        if(ch1==0){
          dup2(pipes[i-1][0],0);
          dup2(pipes[i][1],1);
          execvp(myargs[i][0],myargs[i]);
          printf("exec error2\n");
        }
        else{
          waitpid(ch1,&status,0);
        }
    }
    else
      {
        pid_t ch1;
        ch1=fork();
        if(ch1==0){
            dup2(pipes[i-1][0],0);
            // argno = 0;
            // while(myargs[i][argno] != NULL){
            // 	printf("{%s}\n", myargs[i][argno]);
            // 	argno++;
            // }
          execvp(myargs[i][0],myargs[i]);
          printf("exec error3\n");
        }
        else{
          waitpid(ch1,&status,0);
        }
      }
    
    }
    close(pipes[0][0]);
    close(pipes[0][1]);
    close(pipes[1][0]);
    close(pipes[1][1]);
	return;
}
void sigquit_overloaded(){
	interrupt = 1;
	printf("\nEnter the character or string(without spaces) which you want to search for \n");
	char c[100] ;
	char* req = (char*)malloc(sizeof(char)*100);
	int i= 0 ;
	gets(c);	
	FILE *fp ;
	fp = fopen(blah,"r");
	char *line = NULL;
	size_t len; 
	ssize_t reader; 
	while ((reader = getline(&line, &len, fp)) != -1) {
		char *temp = (char*)malloc(sizeof(char)*100);
		int flag = 1 ;
		i = 0 ;
		int j = 0 ; 
		while(line[i] != '\0'){
			if(line[i] == ' '){
				i++;
				break;
			}
			i++;
		}
		j = 0;
		while(line[i] != '\0'){
			temp[j] = line[i] ;
			j++ ; i++ ;
		}
		i = 0;
		while(c[i] != '\0'){
			if(temp[i] != c[i])
				flag = 0 ;
			i++ ;
		}
		printf("%s\n",temp );
		if(flag == 1)
			strcpy(req,temp) ;

	}
	
	printf("%s\n",req);
	fclose(fp) ;
}
	

void getinput(char* first, char* keyword, char* last, int *len1, int*len2 , int* len3, int*fileflag){
		int i =0, j=0, flag = 0, k=0;
		fileflag[0] = fileflag[1] = fileflag[2] = -1 ;
		for(i=0; (first[i]=getchar())!='\n'; i++)
		{
			if(first[i]!=' ' && flag !=1)
				{
					keyword[j] = first[i];
					j++;
				}
				else if (first[i]==' ')
				{
					flag = 1;
				}
				else if(first[i]!=' ' && flag==1)
				{
					last[k] = first[i];
					k++;
				}
		}	
		keyword[j] = '\0';
		first[i]='\0';
		last[k]='\0';
		if(strstr(keyword,"|") || last[0] == '|')
			fileflag[0] = 1 ;
		else if(strstr(keyword,"<") || last[0] == '<')
			fileflag[1] = 1 ;
		else if(strstr(keyword,">") || last[0] == '>')
			fileflag[2] = 1 ;
		*len1 = j ;
		*len2 = k ;
		*len3 = i ;
}




int main(int argc, char *argv[], char **envp) {
	signal(SIGQUIT,sigquit_overloaded);
	char cwd[1024] ;
	FILE *fp ;
	int offset =1,flag = 0 , len1, len2 , len3, k=0, fileflag[3];
	fp = fopen("Command_List.txt", "a+");
	char *line = NULL;
	size_t len; 
	ssize_t reader; 
	fscanf(fp,"%d %s",&offset,blah);
	if(offset==0 && blah[0] != '4'){
		fprintf(fp,"\n");
		fclose(fp);
		fp = fopen("Command_List.txt", "a+");
	} 
	
	else{

		while ((reader = getline(&line, &len, fp) != -1))  {
    	      
   	    }

    	if(line != NULL){
    		offset = 0;
	    	while(line[k] != ' ' && line[k] >= '0' && line[k] <= '9'){
	    		offset *= 10 ;
	    		offset += (line[k] - '0');
	    		k++ ;
	    	}
		}
	}

	fclose(fp) ;
	 	
	char *first = (char*) malloc(sizeof(char)*64); 
	char *keyword = (char*) malloc(sizeof(char)*64);
	char *last = (char*) malloc(sizeof(char)*64);
	getcwd(cwd, sizeof(cwd)) ;
	strcpy(blah,cwd);
	strcat(blah,"/Command_List.txt");
	//printf("%s", blah);
	
	while(1){
		interrupt = 0;
		offset++;
		getcwd(cwd, sizeof(cwd)) ;
		printf("\n%s>",cwd);
		getinput(first,keyword,last,&len1,&len2,&len3,fileflag);
		fp = fopen(blah, "a+");
		fprintf(fp,"%d %s\n",offset, first) ;
		fclose(fp) ;
		if(strcmp(keyword, "clear") == 0){
			if(last[0] != '\0'){
				printf("No Command '%s' found\n", first) ;
			}
			else{
				
				system("clear");
			}
		}
		else if(strcmp(keyword, "env") == 0){
			if(last[0] != '\0'){
				printf("No Command '%s' found\n", first) ;
			}
			else{
				char **env;
				for(env=envp;*env !=0; env++)
				{
					char *thisEnv = *env;
					printf("%s\n", thisEnv);
				}
			}
		}
		else if(strcmp(keyword, "cd") == 0){
			int ret;
			ret = chdir(last);				
			if(ret == -1){
				perror("The following error(s) occured.");
			}
			else{
				getcwd(cwd, sizeof(cwd));
				printf("%s\n",cwd);
			}
		}
		else if(strcmp(keyword, "pwd")==0){
			getcwd(cwd, sizeof(cwd));
			printf("%s\n",cwd);
		}
		else if(strcmp(keyword, "mkdir")==0){
			int ret;
			ret = mkdir(last, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if(ret == -1){
				perror("The following error(s) occured.");
			}
		}
		else if(strcmp(keyword, "rmdir")== 0){
			int ret;
			ret = rmdir(last);
			if(ret == -1){
				perror("The following error(s) occured.");
			}
		}	
		else if(strcmp(keyword, "ls") == 0) {
			if(last[0] != '\0' && last[0] == '-' && last[1] == 'l'){
				struct stat fileStat;
			    DIR *dp;
			    struct dirent *sd; 
			    char path[1024];
			    size_t ss;
			    getcwd(path,sizeof(path));
			    dp = opendir(path);
			    if(stat(path,&fileStat) < 0)    
			      return 1;

			    while((sd=readdir(dp))!=NULL){
			    	char temp1[1024];
			    	int i=0;
			    	int l2=strlen(path);
			    	for(i=0;i<=l2;i++){
			    	  temp1[i]=path[i];
			    	}
			    	for(i=0;i<=strlen(sd->d_name);i++){
			      		temp1[i]=sd->d_name[i];
			    	}
			    	//printf("%s..%s\n",temp1,sd->d_name);
				    if(stat(temp1,&fileStat) < 0)    
				      continue;
			      	struct passwd *pwuser;
			    	struct group *grpname;
				    if(NULL==(pwuser=getpwuid(fileStat.st_uid))){
				        perror("shell");
				        continue;
				    }
				    if(NULL==(grpname=getgrgid(fileStat.st_gid))){
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
			      	printf("  %lu",fileStat.st_nlink);
			      	printf("  %s",pwuser->pw_name);
			      	printf("  %s",grpname->gr_name);
			      	printf("  %ld",fileStat.st_size);
			      	printf("   %d",ctime(&fileStat.st_atim.tv_sec));
			      	printf("   %s\n",sd->d_name);
			    }
			    
  			}
  			else if(last[0] =='\0'){
				DIR *dir;
				struct dirent *ent;
				if ((dir = opendir (cwd)) != NULL) {
	  				/* print all the files and directories within directory */
	  				while ((ent = readdir (dir)) != NULL) {
	    				printf ("%s\t", ent->d_name);
	  				}
	  				closedir (dir);
				} 
				else {
					perror ("");
				}
			}
			else{
				printf("No Command '%s' found\n", first) ;
			}
		}
		else if(strcmp(keyword,"history") == 0){
			int num = 0;
			if(last[0] != '\0'){
				int i = 0;
				for(;last[i] !='\0';i++){
					if(last[i] <'0' || last[i] > '9')
						printf("No Command '%s' found\n", first) ;
					else{
						num *= 10 ; 
						num += (last[i] -'0');
						
					}
				}
				char command[100] ;
				int off=0 ;

				fp = fopen("Command_List.txt","r");
				while ((reader = getline(&line, &len, fp)) != -1) {
					// printf("In here\n");
					if(offset-off <= num)
	           			printf("%s", line);
	           		off++;
      			}
			}
			else{
			char command[100] ;
			int off ;

			fp = fopen("Command_List.txt","r");
			char *line = NULL;
			size_t len; 
			ssize_t read; 
	   		while ((read = getline(&line, &len, fp)) != -1) {
           		printf("%s", line);
      			}
			}
		}
		else if(strcmp(keyword, "exit")==0){
			return 0;		
		}
		else if(interrupt != 1){
			// if(fileflag[0] == 1){
			// 	int p[2] ,child;
				
			// 	// if((child=fork())==0){
			// 	printf("%s\n",first );
			// 	int std_in = dup(STDIN_FILENO);
			// 	int std_out = dup(STDOUT_FILENO);
			// 	char first_arg[100]; 
			// 	char c[1024] ;
			// 	strcpy(c,cwd);
			// 	strcat(c,"/");
			// 	int i,j=0;
			// 	int firstone = 1; 
			// 	pipe(p) ;
			// 	char b[1024] ;
			// 	strcpy(b,c);
			// 	for(i=0;i<len3;i++){
			// 		// printf("%d %d\n",i,len3 );
			// 		if(first[i] != '|' && first[i] != ' ' ){
			// 			first_arg[j] = first[i];
			// 			j++ ;
			// 		}
			// 		if(first[i] == '|' || (i==len3-1)){
			// 			// printf("%s\n",first_arg );
			// 			if(fork()==0){
			// 				int ret ;
			// 				strcat(b,first_arg);
			// 				if(firstone == 1){
			// 					printf("ENter the first Input\n");
			// 				}
			// 				if(firstone != 1){
			// 					close(0) ;
			// 					dup(p[0]);
			// 					close(p[1]);
			// 					close(p[0]);
			// 				}
							
			// 				if(i == len3-1)
			// 					dup2(std_out,STDOUT_FILENO);
			// 				else{
			// 					close(1);
			// 					dup(p[1]) ;
			// 					close(p[0]);
			// 					close(p[1]);
			// 				}
			// 				// printf("YO1\n");
			// 				// ret = execl(b,first_arg,NULL,(char *)NULL);
			// 				ret = execlp(first_arg, first_arg, NULL , (char*)NULL);
			// 				// printf("YO2\n");
			// 				if(ret == -1){
			// 					ret = 0 ;
			// 					// printf("YO3\n");
			// 						ret = execl(b,first_arg,NULL,(char *)NULL);
			// 						// printf("YO4\n");
			// 						if(ret == -1)
			// 							perror("The following error(s) occured in the current directory.");
			// 						// return 0;
			// 				}
			// 				// printf("YO5\n");
							
			// 				// printf("Exiting the | loop\n");
			// 				return 0 ;
			// 			}
			// 			wait(&child);
			// 			firstone = 0 ;
			// 			j = 0;	
			// 			// break;
			// 		}
					
					
						
			// 	}
			// close(p[0]);
			// close(p[1]);	
			// }
			if(fileflag[0] = 1){
				parsepipe(first);
			}	
			else if(fileflag[2] == 1){
				char first_arg[100], second_arg[100];
					int i, j =0, k= 0;
					flag = 0 ;
					for(i=0;i<len3;i++){
						if(first[i] != ' ' && first[i] != '>' && flag == 0){
							first_arg[k] = first[i] ;
							k++ ;
						}
						else if(first[i] == '>'){
							flag = 1 ;
						}
						else if(first[i] != ' ' && flag == 1){
							second_arg[j] = first[i];
							j++; 
						}
					}
					first_arg[k] = '\0';
					second_arg[j] = '\0';
					int piper[2] ,ret,fd;
					char c[1024],d[1024];
					getcwd(c,sizeof(c));
					strcat(c,"/");
					strcpy(d,c);
					strcat(d,second_arg) ;
					pipe(piper);
					int child ;
					if((child = fork()) == 0){
						close(1) ;
						fd = open(d,O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
						strcat(c,first_arg);
						ret = execlp(first_arg, first_arg, NULL , (char*)NULL);
						if(ret == -1){
							ret = 0 ;
							ret = execl(c,first_arg,NULL,(char *)NULL);
							if(ret == -1)
								perror("The following error(s) occured in the current directory.");
							 
						}
						return 0 ;
					}
					wait(&child) ;
			}
			else if(fileflag[1] == 1){
					char first_arg[100], second_arg[100];
					int i, j =0, k= 0;
					flag = 0 ;
					for(i=0;i<len3;i++){
						if(first[i] != ' ' && first[i] != '<' && flag == 0){
							first_arg[k] = first[i] ;
							k++ ;
						}
						else if(first[i] == '<'){
							flag = 1 ;
						}
						else if(first[i] != ' ' && flag == 1){
							second_arg[j] = first[i];
							j++; 
						}
					}
					first_arg[k] = '\0';
					second_arg[j] = '\0';
					int piper[2] ,ret,fd;
					char c[1024],d[1024];
					getcwd(c,sizeof(c));
					strcat(c,"/");
					strcpy(d,c);
					strcat(d,second_arg) ;
					pipe(piper);
					//dup(piper[0]);
					int child ;
					if((child = fork()) == 0){
						close(0) ;
						if(fd = open(d,O_RDONLY,S_IRUSR | S_IRGRP | S_IROTH) == -1){
							printf("Cannot open the given file for input\n");
							return 0 ;
						}
						strcat(c,first_arg);
						ret = execlp(first_arg, first_arg, NULL , (char*)NULL);
						if(ret == -1){
							ret = 0 ;
							ret = execl(c,first_arg,NULL,(char *)NULL);
							if(ret == -1)
								perror("The following error(s) occured in the current directory.");
							 
						}
						return 0 ;
					}
					wait(&child) ;
				}
			else{
				int child,ret,id ;
				char c[1024];
				getcwd(c,sizeof(c));
				strcat(c,"/");
				strcat(c,keyword);
				if(fork() == 0){
					if(last[len2-1] == '&')
						last[len2-1] = '\0' ;
					ret = execlp(keyword, keyword, last , (char*)NULL);
					if(ret == -1){
						ret = 0 ;
						ret = execl(c,keyword,last,(char *)NULL);
						if(ret == -1)
							perror("The following error(s) occured in the current directory.");
						else{
							getcwd(c,sizeof(c));
							printf("%s>",c);
						} 

					}
					else{
							getcwd(c,sizeof(c));
							printf("%s>",c);
						} 
				}
				else{
					if(last[len2-1] != '&')
						wait(&child);
				
				}
			}
		}
	}
	return 0 ;
}