#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "bank.h"

void deposit(struct account* account, float * amount)
{
	account->bal += (*amount);
}

int withdraw(struct account* account, float * amount)
{
	if (account->bal < (*amount))
		return -1;
	else
		account->bal -= (*amount);
	return 1;
}

struct bank * create(char * name, float * bal)
{
	/*int i = 0;*/
	return 0;
}

void serve(struct account * account)
{
	
}

float * query(struct account * account)
{
	/*mutex lock already in place*/
	float * ret = (float *)(malloc (sizeof (float)));
	*ret = account->bal;
	return ret;
}
