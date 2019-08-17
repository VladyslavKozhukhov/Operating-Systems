#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
int checkKindOfProcess(int count,char ** lst);
int regularProcess(int count , char ** lst);
int backBackgourndProcess(int count,char ** lst,int maxLen);
int pipeProcess(int count,char ** lst,int maxLen,int index);
int max_len_word(char** lst,int count);
static int keepRunning=0;
void dadhandler(int dummy);
int checkIfPipe(int count,char **lst);
void handler(int sig);



// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should cotinue, 0 otherwise
int process_arglist(int count, char** arglist);

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void){
	signal(SIGCHLD,SIG_IGN);//kill zombie
	signal(SIGINT,dadhandler);
	return 0;
}
int finalize(void){
	return 0;
}

/**
 * Regular process
 * @param count - number of words
 * @param lst - command
 * @return  1
 */
int regularProcess(int count , char ** lst){
	int status =0;
	pid_t pid = -1;
	//signal(SIGINT,intHandler);
	pid=fork();
	if(pid < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0){
		while(keepRunning){
		if(execvp(lst[0],lst)<0){
					printf("%s: command not found \n",lst[0]);
					exit(-1);
			}
		}
		if(getpgid(pid) <0)
			exit(1);
	}
	else if(pid>0){		
				while(waitpid(pid,&status,0)>0);
               //  if(child_id<0){ //no child
                 //  return 0;
              //  }
				keepRunning =1;
			}			
		return 1;
	}
	
/**
 *backBackgourndProcess 
 * @param count - number of words
 * @param lst - command
 * @param maxLen -max len of the word
 * @return  1
 */	
int backBackgourndProcess(int count,char ** lst,int maxLen){
	
	pid_t pid = -1;
	lst[count-1]=NULL;//Remove & from command
	pid=fork();
	if(pid < 0){
				fprintf(stderr, "fork error \n");
		exit(EXIT_FAILURE);
	}
	if(pid == 0){
		signal(SIGINT, SIG_IGN);
			if(execvp(lst[0],lst)<0){
				fprintf(stderr, "command not found \n");
				exit(-1);
			}
		}
	
	else{
		return 1;//dont w8t for child
	}
	
	return 1;
}

/**
 *backBackgourndProcess 
 * @param count - number of words
 * @param lst - command
 * @param maxLen -max len of the word
 * @param index - index of "|"
 * @return  1
 */	
int pipeProcess(int count,char ** lst,int maxLen,int index){
	int pfds[2];
	pid_t pid = 0;
	pid_t spid=0;
	lst[index]= NULL;//remove "|"
	if(pipe(pfds) == -1){
		exit(EXIT_FAILURE);
	}
		pid=fork();
	switch (pid) {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
		break;
    case 0://Child	
		dup2(pfds[1], 1);
		close(pfds[1]);//the other side of the pipe 
		close(pfds[0]);//clean
		if(execvp(lst[0],lst)<0){
				fprintf(stderr, "command not found \n");
				exit(0);
			}
		break;
		default:
			spid = fork();
			
			if(spid < 0){
				fprintf(stderr, "fork \n");
				exit(EXIT_FAILURE);
			}
			
			if(spid == 0){
				dup2(pfds[0], 0);
				close(pfds[0]);
				close(pfds[1]);
			if(execvp(lst[index+1], lst+index+1)<0){
				fprintf(stderr, "command not found \n");
				exit(0);
			}
			_exit(0);				
			}
			else{
				close(pfds[0]);
				close(pfds[1]);
				 waitpid(pid, NULL, 1);
				waitpid(spid, NULL, 1); 
				break;
			}
			
	}	
	return 1;
}

int process_arglist(int count, char** arglist){
	keepRunning=1;
	int kindOfProcess = checkKindOfProcess(count,arglist);//Get Kind Of Process
	int maxLen = max_len_word(arglist,count);//Max Len Of The Longest Word
	switch(kindOfProcess){
		case  -3:
			return regularProcess(count,arglist);
		break;
		case -2:
			return backBackgourndProcess(count,arglist,maxLen);
		break;
		default:
			return pipeProcess(count,arglist,maxLen,kindOfProcess);		
	}
	return 1;
}
	


int checkKindOfProcess(int count,char ** lst){
	int answer= -3;
	int pipeFlag=-2;
	if(lst[count-1][0]=='&'){//Bakground process
		answer = -2;
	}
	else{
		pipeFlag=checkIfPipe(count,lst);//If pipe command||send index of "|"
		if(pipeFlag>-1){
			return pipeFlag;
		}
	}
	return answer;
	
}


int checkIfPipe(int count,char **lst){
	int ans =-1;
	int i=0;
	for(;i<count;i++){
		if(strcmp(*(lst+i),"|")==0){ans=i;}
	}
	return ans;
}

void dadhandler(int sig){
	keepRunning=0;
}
int max_len_word(char** lst,int count){
	int lenmax=-1;
	int i=0;	
	 for(;i<count; i++){
        if(lenmax<strlen(lst[i])){
            lenmax=strlen(lst[i]);
        }
    }
	return lenmax;
}