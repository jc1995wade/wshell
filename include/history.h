#ifndef _HISTORY_H_
#define _HISTORY_H_

#include "common.h"

typedef char  ElementType;

typedef struct hist{
	struct hist *prev;
	struct hist *next;
	ElementType *n;
} HistoryType;

HistoryType  Hist;
/*===========history=============*/
// history
// history  num

void initHistoryList(HistoryType *head);




// save cmd
ElementType *mhistory(ElementType *cmd);


void mSaveHistory(ElementType *cmd);


// display cmd
void mDisplayHistory(int ar, char **av);

#endif
