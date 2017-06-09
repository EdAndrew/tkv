#include "server.h"

int serverPort;

int doFileEvent(int fd) {
    int nr, sum = 0;
    char buf[1024];
    char *pos = buf;

    while ((nr = read(fd, pos, 1024)) > 0) {
        pos += nr;
        sum += nr;
        if (*(pos-1) == '\n') {
            break;
        }
    }
   
    pos = buf; 
    while ((nr = write(fd, pos, sum)) < sum) {
        pos += nr; 
        sum -= nr;
    }
   
    close(fd); 
}

int doTimerEvent() {
    int fd;
    char *word;

    word = "Do timer event.\n";
    fd = open("/home/jiahao/tkv-timer.txt", O_RDWR|O_APPEND|O_CREAT, S_IWUSR|S_IRUSR);
    write(fd, word, strlen(word)); 
    close(fd);
}

int serverInit() {
    int listenFd;
    int optval;
    struct sockaddr_in serverAddr;
   
    /* tkv default port is 6433 */ 
    serverPort = 6433;

    /* Create a socket */ 
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        fprintf(logFile, "Server init fail: socket().\n");
        return -1;
    }
    
    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
        fprintf(logFile, "Server init fail: setsockopt().\n");
        return -2;
    }
    
    /* Bind socket with remote andress and local port */
    bzero((char *)&serverAddr, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons((unsigned short)serverPort);
    if (bind(listenFd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        fprintf(logFile, "Server init fail: bind().\n");
        return -3;
    }

    /* Convert socket to listen socket */
    if (listen(listenFd, LISTENQ) < 0) {
        fprintf(logFile, "Server init fail: bind().\n");
        return -4;
    }

    return listenFd;
}

int serverRun(listenFd) {
    int epfd, fd;
    int nrEvents;
    struct epoll_event event;
    struct epoll_event *events;
    struct timeval startTime, nowTime;
    int outTime;

    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);

    /* Create epoll facility */
    epfd = epoll_create1(0);
    if (epfd < 0) {
        fprintf(logFile, "Server error: epoll_create1().\n");
        goto err1;
    }

    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenFd, &event);

    /* Simple event-driven server, handling request file event and timer event */
    for (;;) {
        outTime = 3000;
        gettimeofday(&startTime, NULL);
    epw:
        nrEvents = epoll_wait(epfd, events, MAXEVENTS, outTime);
        if (nrEvents < 0) {
            fprintf(logFile, "Epoll_wait() error.\n");
            goto err1;
        }

        /* Handle file events */
        if (nrEvents > 0) {
            fd = accept(listenFd, NULL, NULL);
            if (fd < 0) {
                fprintf(logFile, "accept() error.\n");
                goto err1;
            }
            doFileEvent(fd);

            gettimeofday(&nowTime, NULL);
            outTime = ((startTime.tv_sec + 3) * 1000000 + startTime.tv_usec) - (nowTime.tv_sec * 1000000 + nowTime.tv_usec);
            outTime /= 1000;
            if (outTime > 0) {
                goto epw;
            }
        }

        /* Handle timer events */
        doTimerEvent();
    }

    free(events);
    return 0;

err1:
    free(events);
    return -1;
}

