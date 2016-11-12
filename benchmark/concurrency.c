#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <keyvalue.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int i=0, number_of_keys=1024, number_of_transactions = 65536; 
    int a;
    int tid;
    __u64 size;
    char data[4096];
    int devfd;
    FILE* outfile;
    int pid;
    int key;
    int op;
    sem_t *sem1, *sem2;

    if(argc < 3) {
        fprintf(stderr, "Usage: %s number_of_keys number_of_transactions\n",argv[0]);
        exit(1);
    }

    number_of_keys = atoi(argv[1]);
    number_of_transactions = atoi(argv[2]);

    devfd = open("/dev/keyvalue",O_RDWR);
    
    if(devfd < 0) {
        fprintf(stderr, "Device open failed");
        exit(1);
    }

    srand((int)time(NULL)+(int)getpid());
    
    sem1 = sem_open("csc501_test_sync_1", O_CREAT|O_EXCL, 0, 0);
    sem_unlink("csc501_test_sync_1");

    sem2 = sem_open("csc501_test_sync_2", O_CREAT|O_EXCL, 0, 0);
    sem_unlink("csc501_test_sync_2"); 

    if (!sem1 || !sem2) {
        fprintf(stderr, "Failed to create semaphore.\n");
        exit(1);
    }

    outfile = fopen("output1","w");
    if (outfile == 0) {
        fprintf(stderr, "Output open failed");
        exit(1); 
    }

    // Initializing the keys
    for (i = 0; i < number_of_keys; i++) {
        memset(data, 0, 4096);
        a = rand();
        sprintf(data,"%d",a);
        tid = kv_set(devfd,i,strlen(data),data);
        fprintf(outfile,"S\t%d\t%d\t%d\t%s\n",tid,i,strlen(data),data);
    }

    if (fclose(outfile)) {
        fprintf(stderr, "Output1 failed to close\n");
        exit(1);
    }

    pid = fork();
    if (pid == 0) {
        outfile = fopen("output2","w");
        if (outfile == 0) {
            fprintf(stderr, "Output2 open failed");
            exit(1); 
        }
        sem_post(sem1);
        sem_wait(sem2);
    } else if (pid > 0) {
        outfile = fopen("output3","w");
        if (outfile == 0) {
            fprintf(stderr, "Output3 open failed");
            exit(1); 
        }
        sem_post(sem2);
        sem_wait(sem1);
    } else {
        fprintf(stderr, "Fork failed");
        exit(1);
    }

    for (i = 0; i < number_of_transactions; i++) {
        op = rand() % 2;
        key = rand() % number_of_keys;
        memset(data, 0, 4096);
        if (op) {
            tid = kv_get(devfd,key,&size,data);
            fprintf(outfile,"G\t%d\t%d\t%d\t%s\n",tid,key,size,data);
        } else {
            a = rand();
            sprintf(data,"%d",a);
            tid = kv_set(devfd,key,strlen(data),data);
            fprintf(outfile,"S\t%d\t%d\t%d\t%s\n",tid,key,strlen(data),data); 
        }
    }

    if (pid == 0) {
        sem_post(sem1);
        sem_wait(sem2); 
    } else {
        sem_post(sem2);
        sem_wait(sem1); 
        //for (i = 0; i < number_of_keys; i++)
        //{
        //    tid = kv_delete(devfd,i);
        //}
    }

    fclose(outfile);
    close(devfd);
    sem_close(sem1);
    sem_close(sem2);

    return 0;
}

