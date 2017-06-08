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

int test();
#endif
