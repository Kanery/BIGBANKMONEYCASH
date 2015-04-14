typedef struct Account{
	
	char * name;
	float bal;
	int sesFlag;

} acct;
	
typedef struct Bank{
	
	acct[20] accounts;
	pthread_mutex_t[20] acmutex;
	pthread_mutex_t_ bankmutex;
} bank;


void deposit(acct account, float * amount)
{
	
}

void withdraw(acct account, float * amount)
{
	
}
