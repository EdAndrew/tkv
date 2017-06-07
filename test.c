#include "tkv.h"

extern struct KVDict *kvspace;

int test() {
    int flag;
    int len, testLen;
    const char *testValue;
    char value[256];
    int i = 0;
    int testOrder = 0;

    /* Test 1 */
    {
        ++testOrder;
        testValue = "hello world";
        testLen = strlen(testValue);
        setKV(1, testValue, testLen, kvspace);
        getKV(1, kvspace, value, &len);

        flag = !strcmp(testValue, value) && testLen == len;
        printf("Test %d.\n", testOrder);
        if (!flag) {
            printf("Bug: K-V is %d - %s, len is %d, but real K-V is %d - %s, real len is %d\n", 1, testValue, testLen, 1, value, len);
        } else {
            ++i;
        }
        printf("\n");
    }

    /* Test 2 */
    {
        ++testOrder;
        int removeRet, getRet;
        int flag1, flag2;
        testValue = "Begin U(*&(*)(JLKJ End";
        testLen = strlen(testValue);
        setKV(2, testValue, testLen, kvspace);
        getKV(2, kvspace, value, &len);
        flag1 = !strcmp(testValue, value) && testLen == len;

        removeRet = removeKV(2, kvspace);
        getRet = getKV(2, kvspace, value, &len);
        flag2 = removeRet == 0 && getRet == 1;
    
        printf("Test %d.\n", testOrder);
        if (!flag1) {
            printf("Bug: K-V is %d - %s, len is %d, but real K-V is %d - %s, real len is %d\n", 2, testValue, testLen, 2, value, len);
        }
        if (!flag2) {
            printf("Bug: remove test fail, removeKV() return %d, getKV() return %d.\n", removeRet, getRet);
        }
        if (flag1 && flag2) {
            ++i;
        }
        printf("\n");
    }
    
    printf("Report:\n");
    printf("%d/%d pass.\n", i, testOrder);
}
