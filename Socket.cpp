
#include "Socket.h"
#include "Common.h"

#include <unistd.h>
#include <cstring>    

Socket:: Socket(        ) { sock = -1;     }
Socket:: Socket( int fd ) { sock = fd;     }
Socket::~Socket(        ) { close( sock ); }

PortListener::PortListener( int port )
    : Socket( socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) )
{
    if( sock < 0 )
        die( "socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) failed (%d).", sock );

    std::memset( &addr, 0, sizeof addr );
    addr.sin_family      = AF_INET;             // Internet address family.
    addr.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface.
    addr.sin_port        = htons( port );       // Local port.

    if( bind(sock, (sockaddr*)&addr, sizeof addr) < 0 )
        die( "bind(%d) failed.", sock );

    if( listen(sock, 5) < 0 )
        die( "listen(%d,%d) failed.", sock, 5 );
}

Responder::Responder( int fd )
{
    socklen_t s = sizeof addr;
    sock = accept( fd, (sockaddr*)&addr, &s );

    if( sock < 0 )
        die( "accept(%d) failed.", fd );
}

