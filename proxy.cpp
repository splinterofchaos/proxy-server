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


void handle_client(int clntSocket)
{
    const int BUF_SIZE = 256;
    char echoBuffer[BUF_SIZE];

    /* Receive message from client */
    if( recv(clntSocket, echoBuffer, RCVBUFSIZE, 0) < 0 )
        die("recv() failed");

    puts( echoBuffer );

    std::string helloWorldHtml = 
        std::string("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n") +
        "<html> <head> <title>Tutorial: HelloWorld</title> </head> <body> <h1>HelloWorld Tutorial</h1> </body> </html>";
    /* Echo message back to client */
    if( send(clntSocket, helloWorldHtml.data(), helloWorldHtml.size(), 0) != helloWorldHtml.size() )
        die("send() failed");


    close(clntSocket);    /* Close client socket */
}

struct PortListener
{
    int sock;
    sockaddr_in addr;

    PortListener( int port );
};

PortListener::PortListener( int port )
{
    if( (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
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

        /* Wait for a client to connect */
        if ((clntSock = accept(listener.sock, (struct sockaddr *) &echoClntAddr, 
                               &clntLen)) < 0)
            die("accept() failed");

        /* clntSock is connected to a client! */
        printf( "Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr) );

        handle_client(clntSock);
    }
    /* NOT REACHED */
}
