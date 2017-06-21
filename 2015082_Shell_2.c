#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>


void launch(){
	int status;
	pid_t pi;
	pi=fork();
	if(pi==0){
		char *temp[]={"gnome-terminal","-e","./a.out",NULL};
		execvp(temp[0],temp);
		exit(1);
	}
	wait(&status);
}

int main(){
	launch();
	return 0;
}