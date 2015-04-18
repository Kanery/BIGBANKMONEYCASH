#include        <sys/time.h>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <errno.h>
#include        <netdb.h>
#include        <pthread.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <strings.h>
#include        <unistd.h>

#define CLIENT_PORT	53245

void
set_iaddr_str( struct sockaddr_in * sockaddr, char * x, unsigned int port )
{
	struct hostent * hostptr;

	memset( sockaddr, 0, sizeof(sockaddr) );
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons( port );
	if ( (hostptr = gethostbyname( x )) == NULL )
	{
		printf( "Error getting addr information\n" );
	}
	else
	{
		bcopy( hostptr->h_addr, (char *)&sockaddr->sin_addr, hostptr->h_length );
	}
}

int
main( int argc, char ** argv )
{
	int			sd;
	struct sockaddr_in	addr;
	char			string[512];
	char			buffer[512];
	char			prompt[] = "Enter a string>>";
	char			output[512];
	int			len;
	char *			func = "main";

	if ( argc < 2 )
	{
		printf( "\x1b[2;31mMust specify server name on command line\x1b[0m\n" );
		return 1;
	}
	set_iaddr_str( &addr, argv[1], CLIENT_PORT );
	printf( "Trying to connect to server.%s ...\n", argv[1] );
	do {
		errno = 0;
		if ( (sd = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
		{
			printf( "socket() failed in %s()\n", func );
			return -1;
		}
		else if ( connect( sd, (const struct sockaddr *)&addr, sizeof(addr)) == -1 )
		{
			close( sd );
		}
	} while ( errno == ECONNREFUSED );
	if ( errno != 0 )
	{
		printf( "Could not connect to server %s errno %d\n", argv[1], errno );
	}
	else
	{
		printf( "Connected to server %s\n", argv[1] );
		while ( write( 1, prompt, sizeof(prompt) ), (len = read( 0, string, sizeof(string) )) > 0 )
		{
			string[len]= '\0';
			write( sd, string, strlen( string ) + 1 );
			read( sd, buffer, sizeof(buffer) );
			sprintf( output, "Result is >%s<\n", buffer );
			write( 1, output, strlen(output) );
		}
	}
	return 0;
}
