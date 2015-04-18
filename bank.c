#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "bank.h"
#define BAL_CAP 9000000000000000 /*Which means our bank's maximum possible balance amount, for the sake of our program, will be set to this value.*/

int deposit(struct account* acc, float * amount)
{
	if (acc == NULL || amount == NULL)
		return 0;
	else if (*amount < 0)
		return 0;
	else if ((*acc->bal + *amount) > BAL_CAP)
		return 0; /*Balance will exceed our balance cap.*/ 
	else{
		*acc->bal += (*amount);
		return 1; /*Successfully added amount to balance.*/
	}
}

int withdraw(struct account* acc, float * amount)
{
	if (acc == NULL || amount == NULL)
		return 0;
	else if (*amount < 0)
		return 0;
	else if (*acc->bal < (*amount) || *amount > BAL_CAP){
		return 0; /*User is trying to withdraw more than his balance contains.*/
	}
	else
	{	
		*acc->bal -= (*amount);
		return 1; /*Successful withdrawal.*/
	}
}

struct account * create(char * name, float * bal)
{
	accnt newAcc = (accnt)(calloc (1,sizeof (struct account)));

	newAcc->name = name;
	newAcc->bal = bal;
	newAcc->sesFlag = 0;

	return newAcc;
}

void serve(struct account * account)
{
	account->sesFlag = 1;
}

float * query(struct account * account)
{
	/*mutex lock already in place*/
	float * ret = (float *)(malloc (sizeof (float)));
	ret = account->bal;
	return ret;
}
