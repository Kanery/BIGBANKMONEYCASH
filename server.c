#include 	"bank.h"
#include        <sys/time.h>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <errno.h>
#include        <semaphore.h>
#include        <pthread.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>
#include 	<ctype.h>
#include 	<netdb.h>

#define CLIENT_PORT	53245

/*static pthread_attr_t	user_attr;*/
static pthread_attr_t	kernel_attr;
static sem_t		actionCycleSemaphore;
static pthread_mutex_t	mutex;
static int		connection_count = 0;
static struct bank*  	myBank;
/*static struct account	EmptyAccount = {NULL, 0, 0};*/
static pthread_mutex_t	acMutex[MAX_ACCOUNTS] = 
{
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER,	
	PTHREAD_MUTEX_INITIALIZER
};
static pthread_mutex_t  bankMutex = PTHREAD_MUTEX_INITIALIZER;

static void
set_iaddr( struct sockaddr_in * sockaddr, long x, unsigned int port )
{
	memset( sockaddr, 0, sizeof(*sockaddr) );
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons( port );
	sockaddr->sin_addr.s_addr = htonl( x );
}

static char *
ps( unsigned int x, char * s, char * p )
{
	return x == 1 ? s : p;
}

void
periodic_action_handler( int signo, siginfo_t * ignore, void * ignore2 )
{
	if ( signo == SIGALRM )
	{
		sem_post( &actionCycleSemaphore );		/* Perfectly safe to do ...*/
	}
}

void *
periodic_action_cycle_thread( void * ignore )
{
	struct sigaction	action;
	struct itimerval	interval;

	pthread_detach( pthread_self() );			/* Don't wait for me, Argentina ...*/
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	action.sa_sigaction = periodic_action_handler;
	sigemptyset( &action.sa_mask );
	sigaction( SIGALRM, &action, 0 );			/* invoke periodic_action_handler() when timer expires*/
	interval.it_interval.tv_sec = 20;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 20;
	interval.it_value.tv_usec = 0;
	setitimer( ITIMER_REAL, &interval, 0 );			/* every 20 seconds*/
	for ( ;; )
	{
		sem_wait( &actionCycleSemaphore );		/* Block until posted*/
		pthread_mutex_lock( &mutex );
		printf( "There %s %d active %s.\n", ps( connection_count, "is", "are" ),
			connection_count, ps( connection_count, "connection", "connections" ) );
			
		cyclePrint(myBank->numAccounts);

		pthread_mutex_unlock( &mutex );
		sched_yield();					/* necessary?*/
	}
	return 0;
}

int cyclePrint(int count){
	int i;

	if ( count == 0){
		printf("There are no accounts acrose any banks at this time.\n");
		return 0; /* Nothing to print in bank. */
	}else{
		printf("There %s %d active %s.\n", ps( connection_count, "is", "are" ),
			connection_count, ps( connection_count, "connection", "connections" ) );
		for (i = 0; i < myBank->numAccounts; i++){

			pthread_mutex_lock(&acMutex[i]);

			printf("Client #%d - %s\n", i+1, myBank->accounts[i]->name);
			printf("Available Balance: %g\n", *myBank->accounts[i]->bal);
			if ( myBank->accounts[i]->sesFlag == 1)
				printf("Client %s is currently in session.\n", myBank->accounts[i]->name);
			else
				printf("Client %s is currently not in session.\n", myBank->accounts[i]->name);					

			pthread_mutex_unlock(&acMutex[i]);
			
			printf("-------------------------\n");

		}
		return 1; /* Has successfully printed out contents of accounts in bank. */
	}	
}

void *
client_session_thread( void * arg )
{
	int			sd;
	char			request[2048];
	char			response[2048];
	char			endResponse[2048];
	/*char			temp;*/
	int			i;
	
	/*
	int			limit, size;
	float			ignore;
	long			senderIPaddr;
	char *			func = "client_session_thread";
	*/

	char 			command[100], args[100];
	int 			numArgs;
	struct account *	curAccount;
	struct account *	emptyAcc;
	int			curAcctIndex;
	int			curSessActive;
	float *			temp;
	char * 			tmpStr;

	emptyAcc = (accnt) malloc (sizeof(struct account));
	emptyAcc->name = NULL;
	emptyAcc->bal = NULL;
	emptyAcc->sesFlag = 0;
	curAccount = emptyAcc;
	curAcctIndex = -1;
	curSessActive = 0;

	sd = *(int *)arg;
	free( arg );					/* keeping to memory management covenant*/
	pthread_detach( pthread_self() );		/* Don't join on this thread*/
	pthread_mutex_lock( &mutex );
	++connection_count;				/* multiple clients protected access*/
	pthread_mutex_unlock( &mutex );

	memset(endResponse, '\0', sizeof(endResponse));

	while ( read( sd, request, sizeof(request) ) > 0 )
	{
		printf( "server receives input:  %s\n", request );
	
		memset(response, '\0', sizeof(response));
		memset(command, '\0', sizeof(command));
		memset(args, '\0', sizeof(command));

		numArgs = sscanf(request, "%s %s", command, args);
		printf("Received request: %s. #: %d. cmd: %s. arg: %s\n", request, numArgs, command, args);
		if (numArgs == 0 || numArgs > 2)
		{
			strcpy(response, "Wrong command or argument.  Type <help> to get appropriate syntax.");
			write( sd, response, strlen(response) + 1 );
		}
		else if (numArgs == 1)
		{
			for (i = 0; i < strlen(command); i++)
				command[i] = tolower(command[i]);
		
			if (strcmp(command, "help") == 0)
			{
				strcpy(response, "Commands: \n>Create accountname\n\n>Serve accountname\n\n>Deposit amount\n\n>Withdraw amount\n\n>Query\n\n>end\n\n>quit\n");
				write( sd, response, strlen(response) + 1 );
				/*Help code goes here.*/
			}
			if (curSessActive)
			{
				if (strcmp(command, "query") == 0)
				{
					pthread_mutex_lock(&acMutex[curAcctIndex]);
					sprintf(response, "You currently have $%f in your account %s.  Thank you for banking with us!\n", *(curAccount->bal), curAccount->name);
					pthread_mutex_unlock(&acMutex[curAcctIndex]);

					/*Query code goes here.*/
				}
				else if (strcmp(command, "end") == 0)
				{
					strcpy(response, "--------------------------------\nThank you for banking with us.\nHave an awesome day!\n(BIG CASH MONEY BANK)\nVisit us at rutgers.cash\n------------------------------------\n");
					curSessActive = 0;
					curAccount->sesFlag = 0;
					curAccount = emptyAcc;
					
					/*End is here*/
				}
				else if (strcmp(command, "quit") == 0)
				{
					strcpy(endResponse, "--------------------------------\nThank you for banking with us.\nHave an awesome day!\n(BIG CASH MONEY BANK)\nVisit us at rutgers.cash\n--Quit Connection\n------------------------------------\n");
					curSessActive = 0;
					curAccount->sesFlag = 0;
					curAccount = emptyAcc;
					write( sd, endResponse, strlen(endResponse) + 1 );
					close(sd);
					
					/*Quit is here*/
				}
				else
					strcpy(response, "Error.  Type help to get it.");
				write( sd, response, strlen(response) + 1 );
			}
			else
			{
				/*if (strcmp(command, "quit")*/
				if (strcmp(command, "quit") == 0)
				{
					strcpy(endResponse, "--------------------------------\nThank you for banking with us.\nHave an awesome day!\n(BIG CASH MONEY BANK)\nVisit us at rutgers.cash\n--Quit session.\n------------------------------------\n");
					curSessActive = 0;
					curAccount->sesFlag = 0;
					curAccount = emptyAcc;
					write(sd, endResponse, strlen(endResponse) + 1);
					close(sd);
					/*Quit is here*/
				}
				else if (strcmp(command, "help") == 0)
				{
					strcpy(response, "Commands: \n>Create accountname\n\n>Serve accountname\n\n>Deposit amount\n\n>Withdraw amount\n\n>Query\n\n>end\n\n>quit\n");
				}
				else 
					strcpy(response, "Not sure what you want.\nType help.  Magical things happen.\n");
				write( sd, response, strlen(response) + 1 );
			}
				
		}
		else if (numArgs == 2)
		{
			for (i = 0; i < strlen(command); i++)
				command[i] = tolower(command[i]);
			
			if (curSessActive)
			{	
				if (strcmp(command, "deposit") == 0)
				{
					temp = (float *) calloc (1,sizeof(float));
					if (sscanf(args, "%f", temp) != 1)
						strcpy(response, "Please enter a valid deposit amount.\nType <help> to get add. info\n");
					else
					{
						pthread_mutex_lock(&acMutex[curAcctIndex]);
						if (deposit(curAccount, temp) == 0)
							strcpy(response, "Deposit amount is too large. It has to be verified.  It cannot be processed at this moment.\n We appreciate your business.\n Contact our investment broker at rutgers.cash.\n");
						else
						{
							sprintf(response, "Deposit successful. Your current balance is %f.\n", *curAccount->bal);
							printf("deposit response is %s..\n", response);
						}
						pthread_mutex_unlock(&acMutex[curAcctIndex]);
					} 
					write( sd, response, strlen(response) + 1 );

				}
				else if (strcmp(command, "withdraw") == 0)
				{		
					temp = (float *) calloc (1,sizeof(float));
					if (sscanf(args, "%f", temp) != 1)
						strcpy(response, "Please enter a valid withdrawal amount.\n");
					else{
						pthread_mutex_lock(&acMutex[curAcctIndex]);				
						if (withdraw(curAccount, temp) == 0){
	
							strcpy(response, "You do not have enough funds to withdraw your specified amount.\n");
						}
						
						else{
	
							sprintf(response, "Withdrawal successful. Your current balance is %f.\n", *curAccount->bal);

						}						

						pthread_mutex_unlock(&acMutex[curAcctIndex]);					
					}
					write( sd, response, strlen(response) + 1 );
				}
				else if (strcmp(command, "serve") == 0)
				{
					/*Since currSess is active, this means that
					we are already in a client session, therefore,
					we should not call serve.*/
					
					strcpy(response, "We are already serving you.  To end this session type end or <help>\n");			
					write( sd, response, strlen(response) + 1 );
				}
				else if (strcmp(command, "create") == 0)
				{
					strcpy(response, "Account cannot be created when in session.\n");			
					write( sd, response, strlen(response) + 1 );
				}
				else
				{
					strcpy(response, "Error- type help to get it.");
					write( sd, response, strlen(response) + 1 );
				}
			}
			else
			{
				if (strcmp(command, "create") == 0)
				{	
					if (myBank->numAccounts >= 1)
						printf("my account name is now %s\n", myBank->accounts[myBank->numAccounts-1]->name);
					else
						printf("No account created yet.\n");
					int found = getAccount(args);
					pthread_mutex_lock(&bankMutex);
					if (strlen(args) > 100)
						strcpy(response, "Please input your name UPTO 100 characters. Thank you!\n");
					else if (myBank->numAccounts >= 20)
					{
						strcpy(response, "We are not accepting any new accounts at this time.  Thank you!\n");
					}	
					else if (found != -2 )
					{
						printf("I have a name %s at the %dth spot for bank->accounts\n", myBank->accounts[found]->name, found);
						printf("I'm getting a value of %d for found.  Explain. \n", found);
						strcpy(response, "Are you trying to access your bank account?  Try serve <account name>\n");
					}
					else
					{		
						
						pthread_mutex_lock(&acMutex[myBank->numAccounts]);
						myBank->accounts[myBank->numAccounts] = (struct account *) calloc (1, sizeof(struct account));
						tmpStr = (char *) calloc (101, sizeof(char));
						strcpy(tmpStr, args);
						myBank->accounts[myBank->numAccounts]->name = tmpStr;
						myBank->accounts[myBank->numAccounts]->bal = (float *) calloc (1, sizeof(float));
						myBank->accounts[myBank->numAccounts]->sesFlag = 0;
						pthread_mutex_unlock(&acMutex[myBank->numAccounts]);
						myBank->numAccounts++;
						printf("myBank->numAccounts is now.. %d\n", myBank->numAccounts);
						strcpy(response, "Account opened name.\n Thank you for your business.\n");
					}
					pthread_mutex_unlock(&bankMutex);
					write( sd, response, strlen(response) + 1 );
				}
				else if (strcmp(command, "serve") == 0)
				{
					int accsub = getAccount(args);
					if (accsub == -2)
						strcpy(response, "Account is not found.\n");
					else if (accsub == -1)
						strcpy(response, "FATAL ERROR.\n");
					else
					{
						curSessActive = 1;
						curAcctIndex = accsub;
						curAccount = myBank->accounts[curAcctIndex];
						pthread_mutex_lock(&acMutex[curAcctIndex]);
						if (curAccount->sesFlag == 1)
						{
							strcpy(response, "Waiting to start customer session...\n");
						}
						else 
						{
							curAccount->sesFlag = 1;
						}
						pthread_mutex_unlock(&acMutex[curAcctIndex]);
					}
					write( sd, response, strlen(response) + 1 );
				}		
				else
				{
					strcpy(response, "Invalid command.  Currently not in session.  Type help to see available options.\n");
					write( sd, response, strlen(response) + 1 );
				}
			}
		}
		else
		{
			strcpy(response, "Not sure what you want.\nType help.  Magical things happen.");
			write( sd, response, strlen(response) + 1 );
		}
		sleep(2);
	}
	write(sd, endResponse, strlen(endResponse) + 1);
	close( sd );
	pthread_mutex_lock( &mutex );
	--connection_count;				/* multiple clients protected access*/
	pthread_mutex_unlock( &mutex );
	return 0;
}

/** Returns -2 on not found
    Returns index on found
    Returns -1 on error

ASSUMPTION:: There is no change of name 
*/
int getAccount(char * name)
{
	int i = 0;
	int found = -2;
	if (name == NULL)
		return -1;
	else
	{
		for (; i < strlen(name); i++)
		{
			name[i] = tolower(name[i]);
		}
		for (i = 0; i < MAX_ACCOUNTS; i++)
		{
			printf("at accounts[%d] i have ... %s\n", i, myBank->accounts[i]->name);
			if (strcmp(myBank->accounts[i]->name, name) == 0)
				found = i;
		}
		return found; /*NOT FOUND*/
	}
				
}

void *
session_acceptor_thread( void * arg )
{
	int			sd;
	int			fd;
	int *			fdptr;
	/*struct sockaddr_in	addr;*/
	struct sockaddr_in	senderAddr;
	socklen_t		ic;
	pthread_t		tid;
	/*int			on = 1;*/
	char *			func = "session_acceptor_thread";

	sd = *(int *)arg;

	pthread_detach( pthread_self() );
	/*
	if ( (sd = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
	{
		printf( "socket() failed in %s()\n", func );
		return 0;
	}
	else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
	{
		printf( "setsockopt() failed in %s()\n", func );
		return 0;
	}
	else if ( set_iaddr( &addr, INADDR_ANY, CLIENT_PORT ), errno = 0,
			bind( sd, (const struct sockaddr *)&addr, sizeof(addr) ) == -1 )
	{
		printf( "bind() failed in %s() line %d errno %d\n", func, __LINE__, errno );
		close( sd );
		return 0;
	}
	else if ( listen( sd, 100 ) == -1 )
	{
		printf( "listen() failed in %s() line %d errno %d\n", func, __LINE__, errno );
		close( sd );
		return 0;
	}
	else
	{*/
		ic = sizeof(senderAddr);
		while ( (fd = accept( sd, (struct sockaddr *)&senderAddr, &ic )) != -1 )
		{
			fdptr = (int *)malloc( sizeof(int) );
			*fdptr = fd;					/* pointers are not the same size as ints any more.*/
			if ( pthread_create( &tid, &kernel_attr, client_session_thread, fdptr ) != 0 )
			{
				printf( "pthread_create() failed in %s()\n", func );
				return 0;
			}
			else
			{
				continue;
			}
		}
		close( sd );
		return 0;
	/*}*/
}

int
main( int argc, char ** argv )
{
	pthread_t	tid;
	char *		func = "server main";
	int 		sockdesc;
	int * 		sdptr;
	int 		i;
	accnt 		emptyAcc = (accnt) calloc (1, sizeof(struct account));
	char		message[256];

	i = 0;
	sdptr = (int *) malloc (sizeof(int));
	emptyAcc->name = "";
	emptyAcc->bal = 0;
	emptyAcc->sesFlag = 0;

	pthread_mutex_lock(&bankMutex);
	myBank = (struct bank *) calloc (1, sizeof(struct bank));
	myBank->accounts = (struct account **) calloc(MAX_ACCOUNTS, sizeof(struct account*));
	myBank->numAccounts = 0;
	for (; i < MAX_ACCOUNTS; i++)
	{
		pthread_mutex_lock(&acMutex[i]);
		myBank->accounts[i] = emptyAcc;
		pthread_mutex_unlock(&acMutex[i]);
	}
	pthread_mutex_unlock(&bankMutex);

	if ( pthread_attr_init( &kernel_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
		return 0;
	}
	else if ( (sockdesc = claim_port( "53245" )) == -1)
	{
		write(1, message, sprintf(message, "\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n", "53245", strerror( errno) ) );\
		return 1;
	}	
	else if ( listen(sockdesc, 100) == -1 )
	{
		printf( "listen() failed in file %s line %d\n", __FILE__, __LINE__ );
		close( sockdesc );
		return 0;
	}
	else if ( sem_init( &actionCycleSemaphore, 0, 0 ) != 0 )
	{
		printf( "sem_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_mutex_init( &mutex, NULL ) != 0 )
	{
		printf( "pthread_mutex_init() failed in %s()\n", func );
		return 0;
	}
	else if ( (*sdptr = sockdesc),  pthread_create( &tid, &kernel_attr, session_acceptor_thread, sdptr ) != 0 )
	{
		printf( "pthread_create() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_create( &tid, &kernel_attr, periodic_action_cycle_thread, 0 ) != 0 )
	{
		printf( "pthread_create() failed in %s()\n", func );
		return 0;
	}
	else
	{
		printf( "\n\t\tSERVER HAS BEEN ESTABLISHED.\n" );
		pthread_exit( 0 );
	}
}

int
claim_port( const char * port )
{
	struct addrinfo		addrinfo;
	struct addrinfo *	result;
	int			sd;
	char			message[256];
	int			on = 1;

	addrinfo.ai_flags = AI_PASSIVE;		/* for bind() */
	addrinfo.ai_family = AF_INET;		/* IPv4 only */
	addrinfo.ai_socktype = SOCK_STREAM;	/* Want TCP/IP */
	addrinfo.ai_protocol = 0;		/* Any protocol */
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if ( getaddrinfo( 0, port, &addrinfo, &result ) != 0 )		/* want port 53245 */
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed errno is %s.  File %s line %d.\x1b[0m\n", port, strerror( errno ), __FILE__, __LINE__ );
		return -1;
	}
	else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
	{
		write( 1, message, sprintf( message, "socket() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
		freeaddrinfo( result );
		return -1;
	}
	else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
	{
		write( 1, message, sprintf( message, "setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
		freeaddrinfo( result );
		close( sd );
		return -1;
	}
	else if ( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
	{
		freeaddrinfo( result );
		close( sd );
		write( 1, message, sprintf( message, "\x1b[2;33mBinding to port %s ...\x1b[0m\n", port ) );
		return -1;
	}
	else
	{
		write( 1, message, sprintf( message,  "\x1b[1;32mSUCCESS : Bind to port %s\x1b[0m\n", port ) );
		freeaddrinfo( result );		
		return sd;			/* bind() succeeded;*/
	}
}
