#define MAX_ACCOUNTS  20

struct account
{
        char * name;
        float bal;
        int sesFlag;
};

struct bank
{
        struct account * accounts;
        pthread_mutex_t* acmutex;
        pthread_mutex_t bankmutex;
	int numAccounts;
};


void deposit(struct account* , float * amount);
int withdraw(struct account* , float * amount);
struct account * create(char * name, float * bal);
void serve(struct account * );
float* query(struct account *);
int cyclePrint(int count);
