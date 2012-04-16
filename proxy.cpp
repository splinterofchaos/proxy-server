
#include "Socket.h"
#include "Common.h"

#include <string>     
#include <cstdio>      
#include <cstdlib>      
#include <cstring>     

void handle_client( Responder& client );
void handle_get( int client, const std::string& message );

// Just a silly wrapper.
bool send( int client, const std::string& str );
bool send( int client, const char* const str, size_t len );
bool send_error( int client, int error, const std::string& note );

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
                   handle_client( client ); 
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

void handle_client( Responder& client )
{
    // To make sure we get the client's whole message, the input must be
    // buffered.
    char buf[125];   // Read buffer.
    std::string msg; // Accumulation buffer.
    int n;           // Temp representing bytes read from recv/send.

    printf( "Client at %s connected.\n", inet_ntoa(client.addr.sin_addr) );

    // The client will always end with "\r\n\r\n".
    do
    {
        n = recv( client.sock, buf, sizeof buf, 0 );
        if( n > 0 )
            msg.append( buf, n );
        else if( n < 0 )
            die( "recv(%d) failed.\nAlready sent:\n\"%s\"", 
                 client.sock, msg.c_str() );

        //printf( "%s", buf );
    } while( n and not ends_with(msg, "\r\n\r\n") );


    n = sscanf( msg.c_str(), "%s", buf );

    if( strcmp(buf, "GET") == 0 )
        handle_get( client.sock, msg );
    else
        send_error( client.sock, 400, buf );
}

bool send( int client, const std::string& str )
{
    return send( client, str.c_str(), str.size() );
}


bool send( int client, const char* const str, size_t len )
{
    return send( client, str, len, 0 ) == len;
}

bool send_error( int client, int error, const std::string& note="" )
{
    char* const message = new char[256];
    const char* errStr = 0;
    const char* errMsg = 0;

    switch( error )
    {
      case 400: errStr = "400 Bad Request"; 
                errMsg = "I don't even know what you wanted.";
                break;
      case 404: errStr = "404 Not Found";   
                errMsg = "Okay, maybe i lost it--i don't know. "
                         "That thing you want? Not here.";
                break;
    }

    sprintf ( 
        message, 
        "HTTP/1.0 %s\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>"
        "<head> <title>%s</title> </head>"
        "<body> <h1>%s</h1> \r\n %s <br><br>"
            " Note: %s</body>"
        "</html>\r\n\r\n",
        errStr, errStr, errStr, errMsg, note.c_str()
    );

    send( client, message );
    printf( "HTTP error '%s': %s\n", errStr, note.c_str() );
}

void handle_get( int client, const std::string& message )
{
    const char* const helloHtml = 
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>"
            "<head> <title>HelloWorld</title> </head>"
            "<body> <h1>HelloWorld </h1>\r\n"
                    "Welcome to CFreenet."   
            "</body>"
        "</html>\r\n\r\n";

    char location[256], http[sizeof "HTTP/x.x" + 1];
    sscanf( message.c_str(), "GET %s %s", location, http );

    printf( "\tRequest: GET %s\n", location );

    if( strcmp(location, "/") == 0 )
    {
        puts( "\t\tSending test HTML." );
        send( client, helloHtml );
        return;
    }

    gchar* contents;
    gsize size;

    // Assume that the / is in relation to the user's home directory. 
    // (Just for now--testing stage.)

    std::string filename = getenv( "HOME" );
    filename.append( location );

    FILE* f = fopen( filename.c_str(), "r" );
    if( f )
    {
        printf( "\t\tFile exists. Sending..." );
        send( client, "HTTP/1.0 200 OK\r\n" );

        char buf[125];
        size_t rd;
        while( (rd=read(fileno(f), buf, sizeof buf)) )
            if( not send(client, buf, rd) )
                die( "Could not send %s to %d.", filename.c_str(), client );

        fclose( f );
        puts( "(done)" );
    }
    else // Requested file doesn't exist.
    {
        send_error( client, 404, filename );
    }
}

