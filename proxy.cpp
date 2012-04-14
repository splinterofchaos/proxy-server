#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <string>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXPENDING 5    /* Maximum outstanding connection requests */

void die(char *errorMessage);  /* Error handling function */
void handle_client(int clntSocket);   /* TCP client handling function */

#define RCVBUFSIZE 256   /* Size of receive buffer */
void die(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}


struct Filefd
{
    int fd;

    Filefd();
    Filefd( int fd );
    ~Filefd(); // Closes fd.

    operator int& (); // returns fd.
};

Filefd:: Filefd(        ) : fd(-1) {}
Filefd:: Filefd( int fd ) : fd(fd) {}
Filefd::~Filefd() { close( fd ); }
Filefd::operator int& () { return fd; }

struct PortListener
{
    Filefd sock;
    sockaddr_in addr;

    PortListener( int port );
};

PortListener::PortListener( int port )
    : sock( socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) )
{
    if( sock < 0 )
        die( "socket(...) failed." );

    memset( &addr, 0, sizeof addr );
    addr.sin_family      = AF_INET;             // Internet address family.
    addr.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface.
    addr.sin_port        = htons( port );       // Local port.

    if( bind(sock, (sockaddr*)&addr, sizeof addr) < 0 )
        die( "bind(...) failed." );

    if( listen(sock, 5) < 0 )
        die( "listen(...) failed." );
}

struct Responder
{
    Filefd sock;
    sockaddr_in addr;

    Responder();

    int accept( int fd );
};

Responder::Responder()
{
}

int Responder::accept( int fd )
{
    socklen_t s = sizeof addr;
    sock = ::accept( fd, (sockaddr*)&addr, &s );
    return sock;
}

int main(int argc, char *argv[])
{
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int clntLen;            /* Length of client address data structure */

    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    PortListener listener( atoi(argv[1]) );

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);

        Responder resp;
        /* Wait for a client to connect */
        if ((clntSock = accept(listener.sock, (struct sockaddr *) &echoClntAddr, 
                               &clntLen)) < 0)
            die("accept() failed");

        switch( fork() )
        {
          case 0: close( listener.sock );
                  handle_client( clntSock );
                  return 0;

          case -1: die( "fork failed." );

          default: close( clntSock );
        }
    }
    /* NOT REACHED */
}

bool ends_with( const std::string& word, const char* const suffix )
{
    return word.rfind( suffix, word.size() - strlen(suffix) ) != std::string::npos;
}

void handle_client( int client )
{
    char buf[125];
    std::string msg;
    int n;

    while( not ends_with(msg, "\r\n\r\n") 
           && ( n = recv(client, buf, sizeof buf, 0) ) > 0 )
        if( n > 0 )
            msg.append( buf, n );

    if( n < 0 )
        die( "recv(...) failed." );

    puts( msg.c_str() );

    std::string helloWorldHtml = 
        "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html>"
            "<head> <title>Tutorial: HelloWorld</title> </head>"
            "<body> <h1>HelloWorld Tutorial</h1>        </body>"
        "</html>\r\n\r\n";
    /* Echo message back to client */
    if( send(client, helloWorldHtml.c_str(), helloWorldHtml.size(), 0) != helloWorldHtml.size() )
        die("send() failed");

    close( client );
}
