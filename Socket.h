
#include <sys/socket.h>
#include <arpa/inet.h> // For sockaddr_in and inet_ntoa().

struct Socket
{
    int sock;
    sockaddr_in addr;

    Socket();
    Socket( int fd );
    ~Socket();
};

struct PortListener : public Socket
{
    PortListener( int port );
};

struct Responder : public Socket
{
    Responder( int fd );
};

