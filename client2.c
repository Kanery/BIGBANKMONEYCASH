#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>

/* Miniature client to exercise getaddrinfo(2).*/

int
connect_to_server( const char * server, const char * port )
{
	int			sd;
	struct addrinfo		addrinfo;
	struct addrinfo *	result;
	char			message[256];

	addrinfo.ai_flags = 0;
	addrinfo.ai_family = AF_INET;		/* IPv4 only */
	addrinfo.ai_socktype = SOCK_STREAM;	/* Want TCP/IP */
	addrinfo.ai_protocol = 0;		/* Any protocol */
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if ( getaddrinfo( server, port, &addrinfo, &result ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed.  File %s line %d.\x1b[0m\n", server, __FILE__, __LINE__ );
		return -1;
	}
	else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
	{
		freeaddrinfo( result );
		return -1;
	}
	else
	{
		do {
			if ( errno = 0, connect( sd, result->ai_addr, result->ai_addrlen ) == -1 )
			{
				sleep( 1 );
				write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
			}
			else
			{
				freeaddrinfo( result );
				return sd;		/* connect() succeeded */
			}
		} while ( errno == ECONNREFUSED );
		freeaddrinfo( result );
		return -1;
	}
}

int
main( int argc, char ** argv )
{
	int			sd;
	char			message[256];
	char			string[512];
	char			buffer[512];
	char			prompt[] = "Enter a string>>";
	char			output[512];
	int			len;
	/*int			count;*/

	if ( argc < 2 )
	{
		fprintf( stderr, "\x1b[1;31mNo host name specified.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( (sd = connect_to_server( argv[1], "53245" )) == -1 )
	{
		write( 1, message, sprintf( message,  "\x1b[1;31mCould not connect to server %s errno %s\x1b[0m\n", argv[1], strerror( errno ) ) );
		return 1;
	}
	else
	{
		printf( "Connected to server %s\n", argv[1] );
		while ( write( 1, prompt, sizeof(prompt) ), (len = read( 0, string, sizeof(string) )) > 0 )
		{
			string[len-1]= '\0';
			write( sd, string, strlen( string ) + 1 );
			read( sd, buffer, sizeof(buffer) );
			sprintf( output, "Result is >%s<\n", buffer );
			write( 1, output, strlen(output) );
		}
		close( sd );
		return 0;
	}
}
