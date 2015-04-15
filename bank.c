#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "bank.h"



void deposit(acct* account, float * amount)
{
	account->bal += (*amount);
}

int withdraw(acct* account, float * amount)
{
	if (account->bal < (*amount))
		return -1;
	else
		account->bal -= (*amount);
	return 1;
}

acct* create(char * name, float * bal)
{
	/*int i = 0;*/
	return 0;
}

void serve(acct * account)
{
	
}

float * query(acct * account)
{
	/*mutex lock already in place*/
	float * ret = (float *)(malloc (sizeof (float)));
	*ret = account->bal;
	return ret;
}

int main (int argc, char ** argv)
{
	return 0;
}
