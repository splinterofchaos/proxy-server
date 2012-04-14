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

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */

    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        die("socket() failed");
      
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        die("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        die("listen() failed");

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, 
                               &clntLen)) < 0)
            die("accept() failed");

        /* clntSock is connected to a client! */
        printf( "Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr) );

        handle_client(clntSock);
    }
    /* NOT REACHED */
}
