#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>

bool ctrl=false;
struct queue *q;
bool in,out;

struct nodeq{
	char val[1000];
	struct nodeq *next;
};

struct queue{
	struct nodeq *head;
	struct nodeq *tail;
};

void loop_pipe(char ***cmd) 
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;
  while (*cmd != NULL)
    {
      pipe(p);
      if ((pid = fork()) == -1)
        {
          exit(0);
        }
      else if (pid == 0)
        {
	        dup2(fd_in, 0);
	        if (*(cmd + 1) != NULL){
	              dup2(p[1], 1);
	        }
	      	close(p[0]);
	      	if(strcmp((*cmd)[0],"help") == 0){
				printf("exit\nhistory\nkill\nmultiple piping\nmultiple redirection\n");
				exit(0);
			}
			else if(strcmp((*cmd)[0],"history") == 0){
						struct nodeq *k = q->tail;
						while(k!=NULL){ 
							printf("%s",k->val);
							k=k->next;
						}
						exit(0);
			 }
			else{
				execvp((*cmd)[0], *cmd);
				printf("Erroneous command\n");
				exit(0);
			}
        }
      else
        {
          wait(NULL);
          close(p[1]);
          fd_in = p[0]; 
          cmd++;
        }
    }
}

void my_handler(int s){
   ctrl=true;
 }

void redirection_handler(char ***cmd){
 	pid_t pid;
 	int j=0;
	if ((pid = fork()) < 0){
		printf("error\n");
	}

	else if (pid == 0)
	{
	    if (in && !out)
	    {
	    	int fd0 = open(*(cmd+1)[0], O_RDONLY);
	        dup2(fd0, STDIN_FILENO);
	        close(fd0);
	        j=0;
	    }

	    else if (out && !in)
	    {
	    	int fd1 = creat(*(cmd+1)[0] , 0644) ;
	        dup2(fd1, STDOUT_FILENO);
	        close(fd1);
	        j=0;
	    }
	    else{
	    	int fd0 = open(*(cmd+1)[0], O_RDONLY);
	        dup2(fd0, STDIN_FILENO);
	        close(fd0);
	        int fd1 = creat(*(cmd+2)[0] , 0644) ;
	        dup2(fd1, STDOUT_FILENO);
	        close(fd1);
	        j=0;	
	    }


	    if(strcmp((*(cmd+j))[0],"help") == 0){
				printf("exit\nhistory\nkill\nmultiple piping\nmultiple redirection\n");
				exit(0);
		}
		else if(strcmp((*(cmd+j))[0],"history") == 0){
						struct nodeq *k = q->tail;
						while(k!=NULL){ 
							printf("%s",k->val);
							k=k->next;
						}	
						exit(0);
			 }
		else{
				execvp((*(cmd+j))[0], *(cmd+j));
				printf("Erroneous command\n");
				exit(0);
			}
	}
	else
	{
	    wait(NULL);
	    in=false;
	    out=false;
	}
 }

void enqueue(struct queue *q, struct nodeq *k){
	if(q->head == NULL && q->tail == NULL){
		q->head = k;
		q->tail = k;
	}
	else{
		q->head->next = k;
		q->head = k;
	}
}

void dequeue(struct queue *q){
	q->tail = q->tail->next;
}


int main()
{
	q = (struct queue*)malloc(sizeof(struct queue));
	if(q == NULL){
		return -1;
	}
	q->head = NULL;
	q->tail = NULL;
	while (1) {
		in=false;
		out=false;
		char command[1000];
		int i,status,count=0;
		char *args[1000];
		struct sigaction sigIntHandler;
   		sigIntHandler.sa_handler = my_handler;
   		sigemptyset(&sigIntHandler.sa_mask);
   		sigIntHandler.sa_flags = 0;
   		sigaction(SIGINT, &sigIntHandler, NULL);
		printf("ush> ");
		fgets(command,sizeof(command),stdin);
		if (strstr(command,"exit")) {
			exit(0);
		}
			
		if(ctrl==false && command[0]!='\n'){
			
			if (strstr(command,"exit")) {
						exit(0);
			}
			char *token;
	    	int l=0;		
			if(count==50){
			dequeue(q);
			struct nodeq *k = (struct nodeq*)malloc(sizeof(struct nodeq));
			strcpy(k->val,strdup(command));
			k->next = NULL;
			enqueue(q, k);		
			}
			else{
				count=count+1;
				struct nodeq *k = (struct nodeq*)malloc(sizeof(struct nodeq));
				strcpy(k->val,strdup(command));
				k->next = NULL;
				enqueue(q, k);	
			}


			int numtoken=0;
	        int rtoken=0;
	        int arr[2],v;
	        int tr=0;
	        for(v=0;v<strlen(command);v++){
	        	if(command[v]=='<' ){
	        		in=true;
	        	}
	        	else if(command[v]=='>'){
	        		out=true;
	        	}
	        }
	        token = strtok(command, "|");
	    	while( token != NULL)
	    	{   args[numtoken]=token;
	    		token = strtok(NULL, "|");
	    		numtoken++;
	        }
	        
	        if(numtoken==1){
				token = strtok(command, "><");
		    	while( token != NULL)
		    	{   args[rtoken]=token;
		    		token = strtok(NULL, "><");
		    		rtoken++;
		        }
	        }
	        int lol=0;
	        int val=numtoken>rtoken?numtoken:rtoken;
			int m,r;
			char *finalargs[100][100];
			char ***cmd = (char***)malloc(sizeof(cmd)*10);
			for(m = 0; m < val; m++) {
				r = 0;
				token = strtok(args[m], " \n");
				while(token != NULL) {
				    finalargs[m][r++] = strdup(token);
				    token = strtok(NULL, " \n");
				}
				l=r;
				finalargs[m][r] = NULL;
			}
			//printf("%d\n",l );
			if(strcmp(finalargs[0][0],"cd") == 0){
						if(l==1){
							chdir("/home");
								
						}
						else if(finalargs[0][1][0]=='/'){
							char path[1000]="";
							int h=1;
							while(h<l){
								strcat(path,finalargs[0][h]);
								h++;
							}
							if(chdir(path)){
								printf("Erroneous command\n");
								//exit(0);
							}
						}
						else{
							char path[1000]="";
							char buff[1000];
							strcpy(path,getcwd(buff,1001));
							int h=1;
							strcat(path,"/");
							while(h<l){
								strcat(path,finalargs[0][h]);
								h++;
							}
							//printf("%s\n",path );
							if(chdir(path)){
								printf("Erroneous command\n");
								//exit(0);
							}	
						}
						
			    }
			else if(numtoken>1 /*&& z==0*/){
					int p = 0;
					for(p = 0; p < m; p++) {
						cmd[p] = finalargs[p];

					}
					cmd[p] = NULL;
					loop_pipe(cmd);
				}
			else if(rtoken>1){
				int p = 0;
				for(p = 0; p < m; p++) {
					cmd[p] = finalargs[p];
					//printf("%s\n", *(cmd[p]));
				}
				cmd[p] = NULL;
				redirection_handler(cmd);
			}
			else if(fork() == 0) {
				if(numtoken==1 && rtoken==1){
						if(strcmp(finalargs[0][0],"history") == 0){
							struct nodeq *k = q->tail;
								while(k->next!=NULL){ //check is history to be printed or not
								printf("%s",k->val);
								k=k->next;
							}
							exit(0);			
			    		}
				    	else if(strcmp(finalargs[0][0],"help") == 0){
							printf("exit\nhistory\nkill\nmultiple piping\nmultiple redirection\n");
							exit(0);
				    	}
				    	else if(strcmp(finalargs[0][0],"kill") == 0){	
							kill(atoi(finalargs[0][1]),SIGKILL);
							exit(0);
				    	}
				    	else{		
				    		execvp(finalargs[0][0],finalargs[0]);
							printf("Erroneous command\n");
							exit(0);
				    	}
				}
			}
			wait(&status);
		}
		else{
			if(ctrl){
				printf("\n");
			}
			ctrl=false;
		}
	}
	return 0;
}
