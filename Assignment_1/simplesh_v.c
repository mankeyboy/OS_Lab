#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
void getinput(char first[50], char keyword[50], char last[50], int *len1, int*len2 , int* len3){
		int i =0, j=0, flag = 0, k=0;
		for(i=0; (first[i]=getchar())!='\n'; i++)
		{
			if(first[i]!=' ' && flag !=1)
			{
				keyword[j++] = first[i];
			}
			else if (first[i]==' ')
			{
				flag = 1;
			}
			else if(first[i]!=' ' && flag==1)
			{
				last[k++] = first[i];
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
	int offset =0 , len1, len2 , len3;
	fp = fopen("Command_List.txt", "rb+");
	do
			{
				fscanf(fp,"%d %s", &offset,blah);
			}
			while( !feof(fp));
	fclose(fp) ;
	offset++; 	
	char first[50], keyword[50], last[50];
	
	while(1){
		getcwd(cwd, sizeof(cwd)) ;
		printf("\n%s>",cwd);
		getinput(first,keyword,last,&len1,&len2,&len3);
		fp = fopen("Command_List.txt", "ab+");
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
		if(strcmp(keyword, "env") == 0){
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
		if(strcmp(keyword, "cd") == 0){
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
		if(strcmp(keyword, "pwd")==0){
			getcwd(cwd, sizeof(cwd));
			printf("%s\n",cwd);
		}
		if(strcmp(keyword, "mkdir")==0){
			int ret;
			ret = mkdir(last, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if(ret == -1){
				perror("The following error(s) occured.");
			}
		}
		if(strcmp(keyword, "rmdir")== 0){
			int ret;
			ret = rmdir(last);
			if(ret == -1){
				perror("The following error(s) occured.");
			}
		}
		if(strcmp(keyword, "ls") == 0) {
			if(last[0] != '\0'){
				// if(last[0] == '-' && last[1] == 'l'){
				// 	DIR *dir;
				// 	struct dirent *ent;
				// 	if ((dir = opendir (cwd)) != NULL) {
	  	// 			/* print all the files and directories within directory */
	  	// 				while ((ent = readdir (dir)) != NULL) {
	   //  					printf ("%u %u %u %u %s\n",ent->d_ino,ent->d_off,ent->d_reclen,ent->d_type, ent->d_name);
	  	// 				}
	  	// 				closedir (dir);
				// 	} 
				// 	else {
				// 		perror ("");
				// 	}
				// }
				// else{
					printf("No Command '%s' found\n", first);	
				// }
			}
			else{
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
		}
		if(strcmp(keyword,"history") == 0){
			if(last[0] != '\0'){
				int num = 0;
				// try{
				// 	num = atoi(last);
				// }
				// catch(...){
				// 	printf("No Command '%s' found\n", first) ;
				// }
				// int num=0,q = 0; 
				// for(;last[q] != '\0';q++){
				// 	num += (int)last[q] ;
				// 	num *= 10 ; 
				//}
				
			}
			else{
				char command[100] ;
				int off ;

				fp = fopen("Command_List.txt","r");
				do
				{
					fscanf(fp,"%d %s", &off,command);
					printf("%d %s \n", off, command);

				}
				while( off < offset && !feof(fp));
			}
		}
		if(strcmp(keyword, "exit")==0){
			exit(0) ;
		}
		else{
			int child,ret ;
			if(fork() == 0){
				ret = execl(cwd,first,last,(char *)0);
				if(ret == -1){
					ret = 0 ;
					ret = execlp(first, first, last , (char*)0);
					if(ret == -1)
						perror("The following error(s) occured.");
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