#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int primeTable[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
