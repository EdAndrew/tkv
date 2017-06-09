#include "server.h"

int serverPort;

int serverInit() {
    int listenFd;
    int optval;
    struct sockaddr_in serverAddr;
   
    /* Create a socket */ 
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        printf("Server init fail: socket().\n");
        return -1;
    }
    
    /* Eliminates "Address already in use" error from bind */
    if (setsocket(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
        printf("Server init fail: setsocket().\n");
        return -2;
    }
    
    /* Bind socket with remote andress and local port */
    bzero((char *)&serverAddr, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons((unsigned short)serverPort);
    if (bind(listenFd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Server init fail: bind().\n");
        return -3;
    }

    /* Convert socket to listen socket */
    if (listen(listenFd, LISTENQ) < 0) {
        printf("Server init fail: bind().\n");
        return -4;
    }

    return listenFd;
}

int serverRun() {

}
