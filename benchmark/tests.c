#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <keyvalue.h>
#include <fcntl.h>

#define PASS 1
#define FAIL 0

int devfd;
int g_tid = -1;
int pass = 1; // 1 - success, 0 - failure

void testSet(int tid, __u64 key, __u64 size, char* data) {
    int t = kv_set(devfd, key, size, data);

    //printf("S\t%d\t%d\t%d\t%s\n",t,key,size,data);

    if (tid < 0) {
        if (t != -1) {
            pass = FAIL;
            //printf("bad -tid\n");
        }
    } else {
        if (g_tid < 0 && tid == 0) {
            g_tid = t;
        } else if (t != g_tid + tid) {
            pass = FAIL;
            //printf("bad tid\n");
        }
    }
}

void testGet(int tid, __u64 key, __u64 size, char* data) {
    int t;
    __u64 s;
    char d[4096];

    t = kv_get(devfd, key, &s, d);

    //printf("G\t%d\t%d\t%d\t%s\n",t,key,s,d);
    
    if (tid < 0) {
        if (t != -1) {
            //printf("bad -tid\n");
            pass = FAIL;
        }
    } else {
        if (g_tid < 0 && tid == 0) {
            g_tid = t;
        } else if (t != g_tid + tid) {
            //printf("bad tid\n");
            pass = FAIL;
        }
        if (size != s || strncmp(data,d,size) != 0) {
            //printf("bad string\n");
            pass = FAIL;
        }
    } 
    
}

void testDelete(int tid, __u64 key) {
    int t = kv_delete(devfd, key);
    if (tid < 0) {
        if (t != -1) {
            pass = FAIL;
        }
    } else {
        if (g_tid < 0 && tid == 0) {
            g_tid = t;
        } else if (t != g_tid + tid) {
            pass = FAIL;
        }
    }
}

void genData(int key, __u64* size, char* data) {
   sprintf(data, "0x%08X", key);
    *size = strlen(data);
}

int main(int argc, char *argv[]) {
    int i=0;
    int a;
    int tid1,tid2;
    __u64 key;
    __u64 size;
    char data[4097];
    int testnum;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Test Number>\n", argv[0]);
        return 1;
    }

    devfd = open("/dev/keyvalue",O_RDWR);
    if(devfd < 0) {
        fprintf(stderr, "Device open failed");
        exit(1);
    }

    srand((int)getpid() + (int)time(NULL));

    testnum = atoi(argv[1]);

    if (testnum == 1) {
        key = rand();
        data[0] = (char)rand();
        testSet(0,key,1,data);
        testGet(1,key,1,data);
        printf("%d\tTest1: Set and Get\n", pass * 40);
    } else if (testnum == 2) {
        key = rand();
        data[0] = (char)rand();
        testSet(0,key,1,data);
        testGet(1,key,1,data);
        testDelete(2,key);
        testGet(-1,key,1,data); 
        printf("%d\tTest2: Delete\n", pass * 20);
    } else if (testnum == 3) {
        key = rand();
        for (i = 0; i < 4097; i++) {
            data[i] = (char)rand();
        }
        testSet(0,key,4096,data);
        testGet(1,key,4096,data); 
        printf("%d\tTest3: Length Limits\n", pass * 5);
    } else if (testnum == 4) {
        key = rand();
        kv_delete(devfd, key);
        testGet(-1,key,1,data);
        testDelete(-1,key);
        printf("%d\tTest4: Key Not Present\n", pass * 10);
    } else if (testnum == 5) {
        int k = 10000;
        memset(data,0, 4097);
        for (i = 0; i < k; i++) {
            genData(i, &size, data);
            testSet(i,i,size,data);
        }
        for (i = 0; i < k; i++) {
            genData(i, &size, data);
            testGet(k+i,i,size,data);
        }
        for (i = 0; i < k; i++) {
            kv_delete(devfd, i);
        }
        printf("%d\tTest5: Scalability\n", pass * 10);
    } else {
        fprintf(stderr, "Invalid test number: %d\n", testnum);
        close(devfd);
        return 1;
    }

    close(devfd);

    return 0;
}

