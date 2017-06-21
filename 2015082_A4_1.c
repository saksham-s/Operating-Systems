#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
//Process numbers start from 0
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t running = PTHREAD_COND_INITIALIZER;
pthread_cond_t ready = PTHREAD_COND_INITIALIZER;
pthread_cond_t main_thread = PTHREAD_COND_INITIALIZER;

bool is_running;
int curr_time,curr_scheduled;
struct proc* process;
char Q[50][100000];
struct config{
	int *Queues;
	int num_queues;
	int boost_time;
	int num_processes;
};

struct config* conf;

struct proc{
	int index;
	int run_inthisqueue;
	int queue;
	int burst_time;
	int start_time;
	int finish_time;

};

void boost(){
	int i;
	for(i=0;i<conf->num_processes;i++){
		if(process[i].queue!=1 && process[i].burst_time!=0 && process[i].queue>1){
			process[i].queue=1;
			process[i].run_inthisqueue=0;
			// printf("Boosted %d\n",i);
		}
 	}
}

int proc_complete(){
	int i;
	for(i=0;i<conf->num_processes;i++){
		if(process[i].burst_time!=0){
			return 0;
		}
	}
	return 1;
}

int getproc(){
	int i;
	for(i=0;i<conf->num_processes;i++){
		if(process[i].start_time==curr_time && process[i].queue==0){
			return i;
		}
	}
	return -1;
}

void init(){
   int i;
   conf=(struct config *)malloc(sizeof(struct config));
   FILE * fp;
   char str1[100],str2[100];
   fp = fopen ("2015082_A4_2.txt", "r");
   rewind(fp);
   fscanf(fp, "%s %s\n", str1,str2);
   fscanf(fp, "%d",&conf->num_queues);
   conf->Queues=(int *)malloc((conf->num_queues+1)*sizeof(int));
   fscanf(fp, "%s %s", str1,str2); 
   for(i=1;i<=conf->num_queues;i++){
   		fscanf(fp,"%d",&((conf->Queues)[i]));
   }
   fscanf(fp, "%s %d", str1,&(conf->boost_time));
   fscanf(fp,"%s %d %s",str2,&(conf->num_processes),str1);
   process=(struct proc *)malloc(sizeof(struct proc)*conf->num_processes);
   for(i=0;i<conf->num_processes;i++){
   		struct proc temp;
   		process[i]=temp;
   		fscanf(fp, "%d %d",&(process[i].burst_time),&(process[i].start_time));
   		process[i].queue=0;
   		process[i].run_inthisqueue=0;
   		process[i].index=i;
   }
   fclose(fp);
   return;
}
bool iaminmain;


void *run(void *to_run){
	pthread_mutex_lock(&mutex1);
	int i,flag=0,turnaround=0;
	struct proc *pro=(struct proc *)to_run;

	// printf("entered run with procid %d\n",pro->index);
	while(pro->burst_time!=0){
		for(i=0;i<conf->num_processes;i++){
			if((process[i].queue<pro->queue && process[i].queue!=0 && process[i].burst_time!=0) || (is_running) || iaminmain){
				i=0;
				// printf("i went in this\n");
				pthread_cond_signal(&main_thread);
				pthread_cond_wait(&ready,&mutex1);
			}
		}
		
		// printf("Currrent time:%d Process id:%d Process q:%d Time in ques:%d Bursst time:%d \n",curr_time,pro->index,pro->queue,pro->run_inthisqueue,pro->burst_time);
		is_running=true;
		while(pro->run_inthisqueue < conf->Queues[pro->queue] && pro->burst_time>0){
			curr_time++;
			pro->burst_time--;
			pro->run_inthisqueue++;
			printf("Currrent time:%d Process id:%d Process queue:%d\n",curr_time,pro->index,pro->queue);
			for(i=1;i<=conf->num_queues;i++){
				if(i==pro->queue){
					char s[10];
					snprintf( s, 10, "%d", pro->index );
					strcat(Q[i],s);
				}
				else{
					strcat(Q[i],"X");
				}
			}	
			if(curr_time % conf->boost_time==0){
				boost();
			}

			if(getproc()!=-1){
				// printf("Going to leave run to create new proc\n");
				pthread_cond_signal(&main_thread);
				if(is_running){
					pthread_cond_wait(&running,&mutex1);
				}
				else{
					iaminmain=true;
					pthread_cond_wait(&ready,&mutex1);
					iaminmain=false;
				}
			}
			if(pro->burst_time==0){
				// printf("Finished process %d\n",pro->index);
				if(turnaround==0){
					turnaround=curr_time;

					pro->finish_time=turnaround;
				}
				is_running=false;
				break;
			}
			if(pro->run_inthisqueue==conf->Queues[pro->queue]){
				if(pro->queue+1<=conf->num_queues){
					// printf("Downgrade q from %d to %d\n",pro->queue,pro->queue+1);
					pro->queue+=1;
					pro->run_inthisqueue=0;
					is_running=false;
					break;
				}
				else{
					is_running=false;
					pro->run_inthisqueue=0;
					break;
				}
			}
		}
		// printf("Signalled main from run at end inside of  outer while loop\n");
		pthread_cond_signal(&main_thread);
		// is_running=false;
		// printf("Going to wait on ready\n");
		pthread_cond_wait(&ready,&mutex1);
	}
	curr_scheduled--;
	if(turnaround==0){
		turnaround=curr_time;
		pro->finish_time=turnaround;		
	}
	pthread_cond_signal(&main_thread);

	pthread_mutex_unlock(&mutex1);
}

void run_proc(){
	int procid=getproc();
	while(procid!=-1){
		curr_scheduled++;
		pthread_t thread;
		process[procid].queue=1;
		pthread_create(&thread, NULL, run,(void *)(&process[procid]));
		procid=getproc();
	}
	return;	
}

int main(){
	pthread_mutex_lock(&mutex1);
	iaminmain=false;
	init();
	// printf("Init done\n");
	int i=0,procid,clag=0;
	is_running=false;
	curr_scheduled=0;
	curr_time=0;
	while(curr_time<process[0].start_time){
		// printf("Incresing curr_time as no process\n");
		curr_time++;//added now
		for(i=1;i<=conf->num_queues;i++){
			strcat(Q[i],"X");
		}
	}
	while(!proc_complete()){
		procid=getproc();
		while(procid!=-1){
			// printf("Creted thread for process %d\n",procid);
			curr_scheduled++;
			pthread_t thread;
			process[procid].queue=1;
			pthread_create(&thread, NULL, run,(void *)(&process[procid]));
			procid=getproc();
		}
		// iaminmain=false;
		if(is_running){
			// printf("Entered is running in main\n");
			pthread_cond_signal(&running);//some other thread getting scheduled
		}
		else if(curr_scheduled!=0){

			// printf("Entered curr schedlued !=0 in main\n");
			pthread_cond_signal(&ready);
		}
		else{

			// printf("Trying to incresee time to find new proc\n");
			int min=INT_MAX;
			clag=0;
			for(i=0;i<conf->num_processes;i++){
				if(process[i].start_time>curr_time && process[i].start_time<min){
					clag=1;
					min=i;
				}
			}
			if(clag){
				while(curr_time<process[min].start_time){
					for(i=1;i<=conf->num_queues;i++){
						strcat(Q[i],"X");
					}
					curr_time++;

				}
				clag=0;
				// printf("Creted new proc after inc time\n");
				run_proc();

			}
			else{

				// printf("All proc finished bye\n");
				pthread_mutex_unlock(&mutex1);			
				return 0;
			}
		}
		// printf("Going to wait in main\n");
		pthread_cond_wait(&main_thread,&mutex1);
		// printf("wake up in main\n");
	}
	// printf("Bye\n");
	pthread_mutex_unlock(&mutex1);
	for(i=0;i<conf->num_processes;i++){
		printf("Process number:%d Turnaround Time:%d\n",i,process[i].finish_time - process[i].start_time);
	}
	for(i=1;i<=conf->num_queues;i++){
		printf("Queue %d:\n%s\n",i,Q[i]);
	}
	return 0;
}
