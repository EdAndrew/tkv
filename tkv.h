#ifndef __TKV_H
#define __TKV_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/limits.h>
#include "server.h"

extern FILE *logFd;

struct KVEntry {
    int key;
    int len;
    char *value;
};

struct KVDict {
    struct KVEntry **head;
    int size;
    int (*hash)(int, int);
};

extern const int primeTable[];
int setKV(int _key, const char *_value, int _len, struct KVDict *_dict);
int getKV(int _key, struct KVDict *_dict, char *retValue, int *retLen);
int removeKV(int _key, struct KVDict *_dict);

int test();
#endif
