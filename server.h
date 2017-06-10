#ifndef __SERVER_H
#define  __SERVER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>

#define LISTENQ 128
#define MAXEVENTS 64

extern int serverPort;
extern FILE *logFile;
extern struct KVDict *kvspace;
int serverInit();
int serverRun();
#endif
