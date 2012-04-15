
#include "Socket.h"
#include "Common.h"

#include <string>     
#include <cstdio>      
#include <cstdlib>      
#include <cstring>     

// For use in handle_get.
#include <glib.h>        
#include <glib/gstdio.h>  

void handle_client( int client );
void handle_get( int client, const std::string& message );

int main(int argc, char *argv[])
{
    if (argc != 2)
        die( "Usage: %s <port>.", argv[0] );

    PortListener listener( std::atoi(argv[1]) );

    while( true )
    {
        Responder client( listener.sock );

        switch( fork() )
        {
          case  0: close( listener.sock ); 
                   handle_client( client.sock ); 
                   std::exit( 0 );

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
        handle_get( client, msg );
    else
        fprintf( stderr, "Unknown request, '%s'.\n", buf );
}

bool send( int client, const std::string& str )
{
    int n = send( client, str.c_str(), str.size(), 0 );
    if( n < 0 )
    {
        fprintf( stderr, "send(%d, %s) failed.\n", 
                 client, str.c_str() );
        n = 0;
    }
    return n;
}

void handle_get( int client, const std::string& message )
{
    const char* const helloHtml = 
        "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html>"
            "<head> <title>HelloWorld</title> </head>"
            "<body> <h1>HelloWorld </h1>\r\n"
                    "Welcome to CFreenet."   
            "</body>"
        "</html>\r\n\r\n";

    char location[256], http[sizeof "HTTP/x.x" + 1];
    sscanf( message.c_str(), "GET %s %s", location, http );

    if( strcmp(location, "/") == 0 )
    {
        send( client, helloHtml );
        return;
    }

    gchar* contents;
    gsize size;

    // Assume that the / is in relation to the user's home directory. 
    // (Just for now--testing stage.)
    gchar* filename = g_build_filename( g_get_home_dir(), location, 0 );
    if( not g_file_get_contents(filename, &contents, &size, 0) )
        die( "Requested file, %s, doesn't exist or can't be read.", filename );

    if( send(client, contents, size, 0) != size )
        die( "Could not send %s to %d", filename, client );

    g_free( contents );
    g_free( filename );
}

