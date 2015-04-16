#define MAX_ACCOUNTS  20
#define BAL_CAP 9000000000000000

struct account
{
        char * name;
        float bal;
        int sesFlag;
};

typedef struct account * acct;

struct bank
{
        acct * accounts;
	int numAccounts;
};


void deposit(struct account* , float * amount);
int withdraw(struct account* , float * amount);
struct account * create(char * name, float * bal);
void serve(struct account * );
float* query(struct account *);
int cyclePrint(int count);
int getAccount(char * name);
