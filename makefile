
CPP=g++ -std=c++0x
CFLAGS=$(shell pkg-config --cflags glib-2.0)
LDFLAGS=$(shell pkg-config --libs glib-2.0)

proxyObj=.socket.o .common.o

compile=${CPP} ${CFLAGS}

proxy : proxy.cpp ${proxyObj}
	${compile} proxy.cpp -o proxy ${proxyObj} ${LDFLAGS}

.socket.o : Socket.*
	${compile} -c Socket.cpp -o .socket.o
.common.o : Common.*
	${compile} -c Common.cpp -o .common.o

