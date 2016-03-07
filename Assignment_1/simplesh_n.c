#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
void getinput(char* first, char* keyword, char* last, int *len1, int*len2 , int* len3){
		int i =0, j=0, flag = 0, k=0;
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
		*len1 = j ;
		*len2 = k ;
		*len3 = i ;
}

void clearScreen()
{
  const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO,CLEAR_SCREE_ANSI,12);
}
int main(int argc, char *argv[], char **envp) {
	char cwd[1024],blah[100] ;
	FILE *fp ;
	int offset =0,flag = 0 , len1, len2 , len3, k=0;
	fp = fopen("Command_List.txt", "ab+");
	char *line = NULL;
	size_t len; 
	ssize_t read; 
	while ((read = getline(&line, &len, fp) != -1)&& feof(fp)!= NULL)  {
          
       }

    if(line != NULL){
	    while(line[k] != ' '){
	    	offset += (line[k] - '0');
	    	k++ ;
	    }
	}
		
	fclose(fp) ;
	offset++; 	
	char *first = (char*) malloc(sizeof(char)*64); 
	char *keyword = (char*) malloc(sizeof(char)*64);
	char *last = (char*) malloc(sizeof(char)*64);
	getcwd(cwd, sizeof(cwd)) ;
	strcpy(blah,cwd);
	strcat(blah,"/Command_List.txt");
	//printf("%s", blah);
	
	while(1){
		getcwd(cwd, sizeof(cwd)) ;
		printf("\n%s>",cwd);
		getinput(first,keyword,last,&len1,&len2,&len3);
		fp = fopen(blah, "a+");
		fprintf(fp,"%d %s\n",offset, first) ;
		fclose(fp) ;
		if(strcmp(keyword, "clear") == 0){
			if(last[0] != '\0'){
				printf("No Command '%s' found\n", first) ;
			}
			else{
				clearScreen();
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
			      	printf("  %d",fileStat.st_nlink);
			      	printf("  %s",pwuser->pw_name);
			      	printf("  %s",grpname->gr_name);
			      	printf("  %ld",fileStat.st_size);
			      	printf("   %s",ctime(&fileStat.st_atim.tv_sec));
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
					else
						num += (last[i] -'0');
				}
				char command[100] ;
				int off ;

				fp = fopen("Command_List.txt","r");
				do
				{
					fscanf(fp,"%d %s", &off,command);
					if(offset - off < num)
						printf("%d %s \n", off, command);

				}
				while( off < offset && !feof(fp));
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
		offset++ ;
	}
	return 0 ;
}