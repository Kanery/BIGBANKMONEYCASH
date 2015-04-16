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
/*#include        <synch.h>*/
#include        <unistd.h>
#include 	<ctype.h>
#include 	"bank.h"

#define CLIENT_PORT	53245

static pthread_attr_t	user_attr;
static pthread_attr_t	kernel_attr;
static sem_t		actionCycleSemaphore;
static pthread_mutex_t	mutex;
static int		connection_count = 0;
static struct bank*  	myBank;
static const struct account	EmptyAccount = {NULL, 0, 0};
static pthread_mutex_t*	acMutex;
static pthread_mutex_t  bankMutex;

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
	struct account		curAccount = EmptyAccount;
	int 			curAcctIndex;
	int			curSessActive = 0;

	sd = *(int *)arg;
	free( arg );					/* keeping to memory management covenant*/
	pthread_detach( pthread_self() );		/* Don't join on this thread*/
	pthread_mutex_lock( &mutex );
	++connection_count;				/* multiple clients protected access*/
	pthread_mutex_unlock( &mutex );
	while ( read( sd, request, sizeof(request) ) > 0 )
	{
		printf( "server receives input:  %s\n", request );
	
		memset(response, '\0', sizeof(response));
		memset(command, '\0', sizeof(command));
		memset(args, '\0', sizeof(command));

		/*
		printf("Reqeust is : %s\n", request);

		strcpy(response, request);
		
		printf("Reqeust is : %s\n", request);*/

		numArgs = sscanf(request, "%s %s", command, args);
		/*printf("Received request: %s. #: %d. cmd: %s. arg: %s\n", request, numArgs, command, args);*/
		if (numArgs == 0 || numArgs > 2)
		{
			strcpy(response, "Wrong command or argument.  Type <help> to get appropriate syntax.");
		}
		else if (numArgs == 1)
		{
			for (i = 0; i < strlen(command); i++)
				command[i] = tolower(command[i]);

			if (strcmp(command, "help") == 0)
			{
				strcpy(response, "Commands: \n>Create accountname\n\n>Serve accountname\n\n>Deposit amount\n\n>Withdraw amount\n\n>Query\n\n>end\n\n>quit\n");
				/*Help code goes here.*/
			}
			if (curSessActive)
			{
				if (strcmp(command, "query") == 0)
				{
					strcpy(response, "You gave me query.");
					
					/*Query code goes here.*/
				}
				else if (strcmp(command, "end") == 0)
				{
					strcpy(response, "--------------------------------\nThank you for banking with us.\nHave an awesome day!\n(BIG CASH MONEY BANK)\nVisit us at rutgers.cash\n------------------------------------\n");
					curSessActive = 0;
					curAccount.sesFlag = 0;
					curAccount = EmptyAccount;
					/*End is here*/
				}
				else if (strcmp(command, "quit") == 0)
				{
					strcpy(response, "--------------------------------\nThank you for banking with us.\nHave an awesome day!\n(BIG CASH MONEY BANK)\nVisit us at rutgers.cash\n------------------------------------\n");
					curSessActive = 0;
					curAccount.sesFlag = 0;
					curAccount = EmptyAccount;
					break;
					/*Quit is here*/
				}
				else
					strcpy(response, "Error.  Type help to get it.");
			}
			else
			{
				/*if (strcmp(command, "quit")*/
			}
				
		}
		else if (numArgs == 2)
		{
			for (i = 0; i < strlen(command); i++)
				command[i] = tolower(command[i]);

			
			if (strcmp(command, "deposit") == 0)
			{
				strcpy(response, "Deposits.");			
			}
			else if (strcmp(command, "withdraw") == 0)
			{
				strcpy(response, "Withdraw.");			
			}
			else if (strcmp(command, "serve") == 0)
			{
				strcpy(response, "Serve.");			
			}
			else if (strcmp(command, "create") == 0)
			{
				strcpy(response, "Create.");			
			}
			else
			{
				strcpy(response, "Error- type help to get it.");
			}
		}
		else
		{
			strcpy(response, "Not sure what you want.\nType help.  Magical things happen.");
		}
		
		/*size = strlen( request );
		limit = strlen( request ) / 2;

		for ( i = 0 ; i < limit ; i++ )
		{
			temp = request[i];
			request[i] = request[size - i - 1];
			request[size - i - 1] = temp;
		}*/
		sleep(2);
		write( sd, response, strlen(response) + 1 );
	}
	close( sd );
	pthread_mutex_lock( &mutex );
	--connection_count;				/* multiple clients protected access*/
	pthread_mutex_unlock( &mutex );
	return 0;
}

/** Returns -2 on not found
    Returns index on found
    Returns -1 on error

ASSUMPTION:: 
*/
int getAccount(char * name)
{
	int i = 0;
	if (name == NULL)
		return -1;
	else
	{
		for (; i < strlen(name); i++)
		{
			name[i] = tolower(name[i]);
		}
		for (i = 0; i < myBank->numAccounts; i++)
		{
			if (strcmp(myBank->accounts[i]->name, name) == 0)
				return i;
		}
		return -2; /*NOT FOUND*/
	}
				
}

void *
session_acceptor_thread( void * ignore )
{
	int			sd;
	int			fd;
	int *			fdptr;
	struct sockaddr_in	addr;
	struct sockaddr_in	senderAddr;
	socklen_t		ic;
	pthread_t		tid;
	int			on = 1;
	char *			func = "session_acceptor_thread";

	pthread_detach( pthread_self() );
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
	{
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
	}
}

int
main( int argc, char ** argv )
{
	pthread_t		tid;
	char *			func = "server main";
	int i = 0;

	myBank = (struct bank *) calloc (1, sizeof(struct bank));
	myBank->accounts = (struct account **) calloc(MAX_ACCOUNTS, sizeof(struct account*));
	myBank->numAccounts = 0;

	if (pthread_mutex_init(&bankMutex, 0) != 0)
	{
		printf("Houston, we have a problem.\n");
		return 1;
	}

	for (i = 0; i < MAX_ACCOUNTS; i++)
	{
		if (pthread_mutex_init(&acMutex[i], 0) != 0)
		{
			printf("Houston, we have a second problem.\n");
			return 1;
		}
	}
	

	if ( pthread_attr_init( &user_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	/*else if ( pthread_attr_setscope( &user_attr, PTHREAD_SCOPE_PROCESS ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
		return 0;
	}*/
	else if ( pthread_attr_init( &kernel_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
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
	else if ( pthread_create( &tid, &kernel_attr, session_acceptor_thread, 0 ) != 0 )
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
		printf( "server is ready to receive client connections ...\n" );
		pthread_exit( 0 );
	}
}
