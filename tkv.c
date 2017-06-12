#include "tkv.h"

const int primeTable[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

FILE *logFile;
struct KVDict * kvspace;

int simpleHash(int key, int size) {
    return key % size; 
}   

int largerPrime(int prime) {
    int i = 0;
    int size = sizeof(primeTable) / sizeof(int);
    while (i < size && prime > primeTable[i]) {
        ++i;
    }
    return primeTable[i];
}

struct KVEntry * initKVEntry(int _key, const char *_value, int _len) {
    struct KVEntry *entry = (struct KVEntry *)malloc(sizeof(struct KVEntry));
    if (entry == NULL) {
        fprintf(logFile, "Init KVEntry fail.\n");
        return NULL;
    }
    entry->key = _key;
    entry->len = _len;
    entry->value = (char *)malloc(sizeof(_len)); 
    if (entry->value == NULL) {
        fprintf(logFile, "Init KVEntry fail.\n");
        return NULL;
    }

    char *des = entry->value;
    while (_len-- && _value != NULL) {
        *des = *_value;
        ++des;
        ++_value;
    }

    return entry;
}

struct KVDict * initKVDict(int _size, int (*_hash)(int, int)) {
    struct KVDict *dict = (struct KVDict *)malloc(sizeof(struct KVDict));
    if (dict == NULL) {
        fprintf(logFile, "Init KVDict fail.\n");
        return NULL;
    }
    dict->hash = _hash;

    int prime;
    prime = largerPrime(_size);
    dict->size = prime;
    dict->head = (struct KVEntry **)malloc(sizeof(struct KVEntry *) * prime);
    if (dict->head == NULL) {
        fprintf(logFile, "Init KVDict fail.\n");
        return NULL;
    }
    int i;
    for (i = 0; i < dict->size; ++i) {
        dict->head[i] = NULL;
    }
    return dict;
}

struct KVEntry * getEntry(int _key, struct KVDict *_dict) {
    int pos;
    pos = _dict->hash(_key, _dict->size); 
    return _dict->head[pos];
}

int setKV(int _key, const char *_value, int _len, struct KVDict *_dict) {
    char *oldValue;
    struct KVEntry *entry;
    entry = getEntry(_key, _dict);
    if (entry == NULL) {
        entry = initKVEntry(_key, _value, _len);
        if (entry == NULL) {
            fprintf(logFile, "SetKV fail.\n");
            return 1;
        }
        _dict->head[_dict->hash(_key, _dict->size)] = entry;
    } else {
        oldValue = entry->value;
        entry->value = (char *)malloc(sizeof(char) * _len);
        if (entry->value == NULL) {
            fprintf(logFile, "SetKV fail.\n");
            return 2;
        }   
        entry->len = _len;
        char *des = entry->value;
        while (_len--) {
            *des = *_value;
            ++des;
            ++_value;
        }
        free(oldValue);
    }
    return 0;
}

int getKV(int _key, struct KVDict *_dict, char *retValue, int *retLen) {
    struct KVEntry *entry;
    entry = getEntry(_key, _dict);
    if (entry == NULL) {
        fprintf(logFile, "No such key.\n");
        return 1;
    }
    if (entry->value == NULL) {
        fprintf(logFile, "Value of K-V entry is empty.\n");
        return 2;
    }   
    
    *retLen = entry->len;
    int i;
    for (i = 0; i < entry->len; ++i) {
        retValue[i] = entry->value[i];
    }
    return 0;
}

int removeKV(int _key, struct KVDict *_dict) {
    struct KVEntry *entry;
    entry = getEntry(_key, _dict);
    if (entry == NULL) {
        fprintf(logFile, "The key is not found.\n"); 
        return 1;
    }
    if (entry->value == NULL) {
        fprintf(logFile, "The value of key-value is NULL.\n"); 
        return 2;
    }

    free(entry->value);
    entry->value = NULL;
    free(entry);
    _dict->head[_dict->hash(_key, _dict->size)] = NULL;
    return 0;
}

int initKVSpace() {
    kvspace = initKVDict(11, simpleHash);
    if (kvspace == NULL) {
        return -1;
    }
    return 0;
}

void help(char *progName) {
    fprintf(logFile, "Usage: %s [-t --test] [-h --help] [-p --port port]\n", progName);
}

int becomeDaemon() {
    int ret;
    int i;
    pid_t pid;
    
    /* Make child process to be adopted into init */
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Daemon fail in fork().\n");
        return 1;
    } else if (pid != 0) {
        exit(EXIT_SUCCESS);
    } 

    /* Create a new session to escape terminate */
    pid = setsid();
    if (pid == (pid_t)-1) {
        fprintf(stderr, "Daemon fail in setsid().\n");
        return 2;
    }

    /* Change working directory */
    ret = chdir("/");
    if (ret != 0) {
        fprintf(stderr, "Daemon fail in chdir().\n");
        return 3;
    }

    /* Close all open file descriptions */
    for (i = 0; i < NR_OPEN; ++i) {
        close(i);
    }

    /* Dedirect stdin, stdout, stderr to /dev/null */
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    return 0;
}

int initLog() {
    char *logPath;
    logPath = "/home/jiahao/tkv.log";
    logFile = fopen(logPath, "a");
    if (logFile == NULL) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    int optionIndex;
    int ret;
    int listenFd;
    static struct option longOptions[] = {
        {"help", no_argument, NULL, 'h'},
        {"test", no_argument, NULL, 't'},
        {"port", required_argument, NULL, 'p'},
        {0, 0, 0, 0}
    };

    /* Set opterr 0 to prevent getopt_long() print error messages to stderr*/
    opterr = 0;
    while ((opt = getopt_long(argc, argv, "htp:", longOptions, &optionIndex)) != -1) {
        switch (opt) {
            case 'h':
                help(argv[0]);
                return 0;
                break;
            case 't':
                initKVSpace();
                test();
                return 0;
                break;
            case 'p':
                fprintf(logFile, "-p %s\n", optarg);
                break;
            case '?':
                help(argv[0]);
                return 1;
                break;
            default:
                help(argv[0]);
                return 1;
        }
    }


    /* Runing as Daemon */
    //if (becomeDaemon()) {
    //    fprintf(logFile, "Daemon fail.\n");
    //    return -1;
    //}

    /* Init log */
    if (initLog() < 0) {
        fprintf(stderr, "Init log fail.\n");
        return -1;
    }

    /* Init key-value space */
    if (initKVSpace()) {
        fprintf(logFile, "Server Init fail.\n");
        return -1;
    }

    /* Init server socket through binding host and port */
    if ((listenFd = serverInit()) < 0) {
        fprintf(logFile, "Server Init fail.\n");
        return -1;
    }
   
    /* Start server loop */ 
    serverRun();

    return 0;
}
