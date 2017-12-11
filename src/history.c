#include "history.h"
/*===========history=============*/
// history
// history  num
#include <stdlib.h>


// 初始化链表
void initHistoryList(HistoryType *head)
{
	head->prev = NULL;
	head->next = NULL;
	head->n = NULL;
}

// save cmd
ElementType *mhistory(ElementType *cmd)
{
	ElementType *tp = NULL;
	if((tp = (ElementType*)malloc(sizeof(cmd))) == NULL){
		fprintf(stderr,"calloc error 1: %s\n", strerror(errno));
		return NULL;
	}
	strcpy(tp, cmd);

	return tp;
}



// 加入链表
void mSaveHistory(ElementType *cmd)
{
	HistoryType *newnode = NULL;
	if(NULL == (newnode = (HistoryType*)calloc(1, sizeof(HistoryType)))){
		fprintf(stderr,"calloc error 2: %s\n", strerror(errno));
		return ;
	}
	newnode->n = mhistory(cmd);
	
	if(Hist.next == NULL){
		Hist.next = newnode;
		newnode->prev = &Hist;
		newnode->next = NULL;
	}
	else{
		newnode->next = Hist.next;
		Hist.next = newnode;

		newnode->prev = &Hist;
		Hist.next->prev = newnode;
	}
	
}



// display cmd
void mDisplayHistory(int ar, char **av)
{
	int i;
	int num;

	if(av[1] == NULL){
		num = 1000;
	}
	else{
		num = atoi(av[1]);
	}
	
	printf("num=%d\n", num);
	HistoryType *node;
	node = &Hist;
	
	if(num < 1 || node->next == NULL){
		printf("node is NULL\n");
		return;
	}
	node = node->next;

	for(i = 0; i < num; i++){
		printf("%-4d%s\n", i, node->n);
		if(node->next == NULL) {
			return;
		}
		node = node->next;
	}
}














