#include "server.h"

int serverPort;

int answer(int fd, const char *buf) {
    char *pos;
    int nr, sum;

    sum = strlen(buf);
    pos = (char *)buf; 
    while ((nr = write(fd, pos, sum)) < sum) {
        pos += nr; 
        sum -= nr;
    }

    return 0;
}

int judgeDigitStr(const char *str) {
    int i;
    for (i = 0; i < strlen(str); ++i) {
        if (!isdigit(str[i])) {
            return -1;
        }
    }
    return 0;
}

int doSet(const char *key, const char *value) {
    int _key, valueLen;

    /* Judge whether key is numeric string */
    if (judgeDigitStr(key)) {
        return -1;
    }

    _key = atoi(key);
    valueLen = strlen(value);

    if (setKV(_key, value, valueLen, kvspace)) {
        return -1;
    }

    return 0;
}

int doGet(const char *key, char *value, int *valueLen) {
    int  _key;

    /* Judge whether key is numeric string */
    if (judgeDigitStr(key)) {
        fprintf(logFile, "doGet() fail: in judgeDigitStr().\n");
        return -1;
    }

    _key = atoi(key);
    if (getKV(_key, kvspace, value, valueLen)) {
        fprintf(logFile, "doGet() fail: in getKV().\n");
        return -1;
    }

    return 0;
}

int doDel(const char *key) {
    int _key;

    if (judgeDigitStr(key)) {
        return -1;
    }

    _key = atoi(key);
    if (removeKV(_key, kvspace)) {
        return -1;
    }

    return 0;
}

int doWrong(int fd) {
    answer(fd, "Format Error\n");
    return 0;
}

int doFileEvent(int fd) {
    int nr, sum = 0;
    char buf[80000];
    char *pos = buf;
    char motion[128], key[128], value[65536];
    int valueLen;
    char *delim = " ", *end = "\r\n", *token;

    /* Clean the buffer */
    memset(buf, 0, sizeof(buf));
    memset(motion, 0, sizeof(motion));
    memset(key, 0, sizeof(key));
    memset(value, 0, sizeof(value));

    /* Recieve client command */
    while ((nr = read(fd, pos, 80000)) > 0) {
        pos += nr;
        sum += nr;
        if (*(pos-1) == '\n') {
            break;
        }
    }

    /* Judge if client close the connecting */
    if (nr == 0) {
        return 1;
    }
   
    /*  Resolve motion from client command */
    token = strtok(buf, delim);
    strcpy(motion, token); 
    
    /* Distinguish motion between SET, GET, DEL */
    if (strcasecmp(motion, "GET") == 0) {
        /* Extract the last sentence and remove final \r\n */
        token = strtok(NULL, delim);
        token = strtok(token, end);
        strcpy(key, token);
        if (doGet(key, value, &valueLen)) {
            doWrong(fd);
            goto err1;
        }
        answer(fd, value);
        answer(fd, "\n");

    } else if (strcasecmp(motion, "SET") == 0) {
        token = strtok(NULL, delim);
        strcpy(key, token);
        token = strtok(NULL, delim);
        token = strtok(token, end);
        /* Avoid segment fault resulting from wrong format like 'SET 1' */
        if (token == NULL) {
            doWrong(fd);
            goto err1;
        }

        strcpy(value, token);
        if (doSet(key, value)) {
            doWrong(fd);
            goto err1;
        }
        answer(fd, "SetOK\n");

    } else if (strcasecmp(motion, "DEL") == 0) {
        token = strtok(NULL, delim);
        token = strtok(token, end);
        strcpy(key, token);
        if (doDel(key)) {
            doWrong(fd);
            goto err1;
        }
        answer(fd, "DelOK\n");

    } else {
        fprintf(logFile, "doFileEvent() fail: Paser fail.\n");
        doWrong(fd);
    }
  
err1: 
    return 0;
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
    int i;
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

    event.data.fd = listenFd;
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
            for (i = 0; i < nrEvents; ++i) {
                /* Recive new request from new clinet via listenFd */
                if (events[i].data.fd == listenFd) {
                    fd = accept(listenFd, NULL, NULL);
                    if (fd < 0) {
                        fprintf(logFile, "accept() error.\n");
                        goto err1;
                    }
                    event.data.fd = fd;
                    event.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
                }
                /* Recive command from connected client */
                else if (events[i].events & EPOLLIN) {
                    fd = events[i].data.fd;
                    if (doFileEvent(fd) == 1) {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
                        close(fd);
                    }
                }
            }

            /* Calculate remaining time for timeout */
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

