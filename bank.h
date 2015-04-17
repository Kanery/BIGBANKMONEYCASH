#define MAX_ACCOUNTS  20
#define BAL_CAP 9000000000000000

struct account
{
        char * name;
        float* bal;
        int sesFlag;
};

typedef struct account * accnt;

struct bank
{
        accnt * accounts;
	int numAccounts;
};


int deposit(struct account* , float *);
int withdraw(struct account* , float *);
struct account * create(char * name, float * bal);
void serve(struct account * );
float* query(struct account *);
int cyclePrint(int count);
int getAccount(char * name);
int claim_port (const char *);
