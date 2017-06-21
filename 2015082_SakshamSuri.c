#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef struct node {
	int size;
	int num;
	int startindex;
	struct node *next;
}node;

int main(){
	char commmand[10];
	int number=0;
	char *memory=(char *)malloc(sizeof(char)*1000000);//each entry represents 1byte
	int k=0;
	for(k=0;k<1000000;k++){
		memory[k]='0';
	}
	int parameter;
	int count=0,j;
	node *head=NULL;
	while(1){
		printf("$:> ");
		scanf("%s",commmand);
		if(!strcmp(commmand,"free")){
			scanf("%d",&parameter);
			if(parameter>number){
				printf("Invalid argument\n");
			}
			else{
				node *m=head;
				if(count>0 && parameter==head->num){
					count--;
					for(j=head->startindex;j<head->startindex+head->size;j++){
						memory[j]='0';
					}
					head=head->next;
					free(m);
				}
				else{
					node *prev;
					node *tem=head;
					while(tem!=NULL){
						if(tem->num==parameter){
							count--;
							for(j=tem->startindex;j<tem->startindex+tem->size;j++){
								memory[j]='0';
							}
							prev->next=tem->next;
							free(tem);
							break;
						}
						prev=tem;
						tem=tem->next;
					}
				}
			}
		}
		else if(!strcmp(commmand,"malloc")){
			scanf("%d",&parameter);
			int flag=0,start;
			int contiguous=0;
			for(k=0;k<(1000000-parameter);k++){
				// printf("%d\n",k);
				contiguous=0;
				for(j=k;j<k+parameter;j++){
					if(memory[j]=='0'){
						contiguous++;
					}
					else{
						break;
					}
				}
				if(contiguous==parameter){
					flag=1;
					start=k;
					for(j=start;j<start+parameter;j++){
						memory[j]='1';
					}
					break;
				}
				// printf("\n");
			}
			if(flag==0){
				printf("Insufficient Memory");
			}
			else{
				number++;
				count++;
				node *newnode=(node *)malloc(sizeof(node));
				newnode->size=parameter;
				newnode->next=NULL;
				newnode->startindex=start;
				newnode->num=number;
				if(head==NULL)
					head=newnode;
				else{
					node *temp=head;
					while(temp->next!=NULL){
						temp=temp->next;
					}
					temp->next=newnode;
				}
			}
		}
		else if(!strcmp(commmand,"print")){
			node *p=head;
			while(p!=NULL){
				printf("Address:%p Size:%d Bytes\n",&memory[p->startindex],p->size);
				p=p->next;
			}
		}
		else if(!strcmp(commmand,"exit")){
			exit(0);
		}
		else{
			printf("Invalid Command\n");
		}
	}
	return 0;
}