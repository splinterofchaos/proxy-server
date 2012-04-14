#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h> // For sockaddr_in and inet_ntoa().
#include <stdlib.h>     
#include <string.h>    
#include <string>     
#include <unistd.h>

#include <stdarg.h> // For va_list (see: die).

struct Filefd;       // RAII file descriptor.
struct PortListener; // Listens to a local port.
struct Responder;    // Responds to a connection.

// Handles simple GET requests.
void handle_client( Responder& client );

void die( char *msg, ... )
{
    va_list args;
    va_start( args, msg );

    vfprintf( stderr, msg, args );
    perror("");

    va_end( args );
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

struct Socket
{
    int sock;
    sockaddr_in addr;

    Socket();
    Socket( int fd );
    ~Socket();
};

Socket:: Socket(        ) { sock = -1;     }
Socket:: Socket( int fd ) { sock = fd;     }
Socket::~Socket(        ) { close( sock ); }

struct PortListener : public Socket
{
    PortListener( int port );
};

PortListener::PortListener( int port )
    : Socket( socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) )
{
    if( sock < 0 )
        die( "socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) failed (%d).", sock );

    memset( &addr, 0, sizeof addr );
    addr.sin_family      = AF_INET;             // Internet address family.
    addr.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface.
    addr.sin_port        = htons( port );       // Local port.

    if( bind(sock, (sockaddr*)&addr, sizeof addr) < 0 )
        die( "bind(%d) failed.", sock );

    if( listen(sock, 5) < 0 )
        die( "listen(%d,%d) failed.", sock, 5 );
}

struct Responder : public Socket
{
    Responder( int fd );
};

Responder::Responder( int fd )
{
    socklen_t s = sizeof addr;
    sock = ::accept( fd, (sockaddr*)&addr, &s );

    if( sock < 0 )
        die( "accept(%d) failed.", fd );
}

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
                   handle_client( client ); // No return.

          case -1: die( "fork failed." );

          default: ; // Do it all over again.
        }
    }
}

bool ends_with( const std::string& word, const char* const suffix )
{
    return word.rfind( suffix, word.size() - strlen(suffix) ) != std::string::npos;
}

void handle_client( Responder& client )
{
    // To make sure we get the client's whole message, the input must be
    // buffered.
    char buf[125];   // Read buffer.
    std::string msg; // Accumulation buffer.
    int n;           // Temp representing bytes read from recv/send.

    // The client will always end with "\r\n\r\n".
    while( not ends_with(msg, "\r\n\r\n") 
           && ( n = recv(client.sock, buf, sizeof buf, 0) ) > 0 )
        if( n > 0 )
            msg.append( buf, n );

    if( n < 0 )
        die( "recv(%d) failed.\nAlready sent:\n\"%s\"", 
             client.sock, msg.c_str() );

    puts( msg.c_str() );

    std::string helloHtml = 
        "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html>"
            "<head> <title>Tutorial: HelloWorld</title> </head>"
            "<body> <h1>HelloWorld Tutorial</h1>        </body>"
        "</html>\r\n\r\n";
    n = send( client.sock, helloHtml.c_str(), helloHtml.size(), 0 );
    if( n != helloHtml.size() )
        die("send(%d,\"%s\") failed.", client.sock, helloHtml.c_str() );

    exit( 0 );
}
