#include "tkv.h"

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
        printf("Init KVEntry fail.\n");
        return NULL;
    }
    entry->key = _key;
    entry->len = _len;
    entry->value = (char *)malloc(sizeof(_len)); 
    if (entry->value == NULL) {
        printf("Init KVEntry fail.\n");
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
        printf("Init KVDict fail.\n");
        return NULL;
    }
    dict->hash = _hash;

    int prime;
    prime = largerPrime(_size);
    dict->size = prime;
    dict->head = (struct KVEntry **)malloc(sizeof(struct KVEntry *) * prime);
    if (dict->head == NULL) {
        printf("Init KVDict fail.\n");
        return NULL;
    }
    int i;
    for (i = 0; i < dict->size; ++i) {
        dict->head[i] = NULL;
    }
    return dict;
}

struct * getEntry(int _key, struct KVDict *_dict) {
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
            printf("SetKV fail.\n");
            return 1;
        }
    } else {
        oldValue = entry->value;
        entry->value = (char *)malloc(sizeof(char) * _len);
        if (entry->value = NULL) {
            printf("SetKV fail.\n");
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
    if (entry == NULL) {
        printf("No such key.\n");
        return 1;
    }
    if (entry->value == NULL) {
        printf("Value of K-V entry is empty.\n");
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
    
    return 0;
}


int init() {
    kvspace = initKVDict(11, simpleHash);
    return 0;
}

int test() {
    int flag;
    int len, testLen;
    const char *testValue;
    char value[256];

    testValue = "hello world";
    testLen = strlen(testValue);
    setKV(1, testValue, strlen(testValue), kvspace);
    getKV(1, kvspace, value, &len);

    int i = 0;
    int testAmount = 1;
    flag = !strcmp(testValue, value) && testLen == len;
    printf("K-V is %d - %s, len is %d, real K-V is %d - %s, len is %d\n", 1, testValue, testLen, 1, value, len);
    if (!flag) {
        printf("Bug: K-V is %d - %s, len is %d, but real K-V is %d - %s, real len is %d\n", 1, testValue, testLen, 1, value, len);
    } else {
        ++i;
    }

    printf("%d/%d test.\n", i, testAmount);
    return 0;
}

int main() {
    init();
    test();
    return 0;
}
