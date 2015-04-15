typedef struct Account
{
        char * name;
        float bal;
        int sesFlag;
} acct;

typedef struct Bank
{
        acct accounts[20];
        pthread_mutex_t acmutex[20];
        pthread_mutex_t bankmutex;
	int numAccounts;
} bank;

void deposit(acct* account, float * amount);
int withdraw(acct* account, float * amount);
acct * create(char * name, float * bal);
void serve(acct * account);
float* query(acct * account);
