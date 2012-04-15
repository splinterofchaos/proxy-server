#include <string>     
#include <cstdio>      
#include <cstring>     

#include "Socket.h"
#include "Common.h"

void handle_client( int client );
void handle_get( int client );

int main(int argc, char *argv[])
{
    if (argc != 2)
        die( "Usage: %s <port>.", argv[0] );

    PortListener listener( atoi(argv[1]) );

    while( true )
    {
        Responder client( listener.sock );

        switch( fork() )
        {
          case  0: close( listener.sock ); 
                   handle_client( client.sock ); 
                   exit( 0 );

          case -1: die( "fork failed." );

          default: ; // Do it all over again.
        }
    }
}

bool ends_with( const std::string& word, const char* const suffix )
{
    return word.rfind( suffix, word.size() - strlen(suffix) ) 
        != std::string::npos;
}

void handle_client( int client )
{
    // To make sure we get the client's whole message, the input must be
    // buffered.
    char buf[125];   // Read buffer.
    std::string msg; // Accumulation buffer.
    int n;           // Temp representing bytes read from recv/send.

    // The client will always end with "\r\n\r\n".
    while( not ends_with(msg, "\r\n\r\n") 
           && ( n = recv(client, buf, sizeof buf, 0) ) > 0 )
        if( n > 0 )
            msg.append( buf, n );

    if( n < 0 )
        die( "recv(%d) failed.\nAlready sent:\n\"%s\"", 
             client, msg.c_str() );

    std::puts( msg.c_str() );

    n = sscanf( msg.c_str(), "%s", buf );
    if( strcmp(buf, "GET") == 0 )
        handle_get( client );
    else
        fprintf( stderr, "Unknown request, '%s'.\n", buf );
}

void handle_get( int client )
{
    std::string helloHtml = 
        "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html>"
            "<head> <title>Tutorial: HelloWorld</title> </head>"
            "<body> <h1>HelloWorld Tutorial</h1>        </body>"
        "</html>\r\n\r\n";
    int n = send( client, helloHtml.c_str(), helloHtml.size(), 0 );
    if( n != helloHtml.size() )
        die("send(%d,\"%s\") failed.", client, helloHtml.c_str() );
}

