#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>


bool mutex_enable = false; 
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
long long int inc_amnt = 1000000;
volatile long long int global_cnt = 0;

void* inc_foo(void* str);
int main (int argc, char ** argv){      
    int opt;
    while((opt = getopt(argc, argv, "Mha:")) != -1){
        switch(opt){
            case 'M':
                mutex_enable = true;
                printf("Running threads blocked by mutex..\n");
                break;
            case 'a':
                inc_amnt = atoi(optarg);
                break;
            case 'h':
                printf("usage : ./a.out \n");
                printf("-M for enabling mutex\n");
                printf("-a <value> for counting till that value (by default 1000000)\n");
                return 0;

        }
    }
    pthread_t thr1, thr2;
    char* str1 = "First thread\n";
    char* str2 = "Second thread\n";

    pthread_create(&thr1, NULL, inc_foo, (void *)str1);
    pthread_create(&thr2, NULL, inc_foo, (void *)str2);
    
    printf("Threads created !\n");

    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL);

    printf(" the global cnt is :%lld\n", global_cnt);
    return 0;
}

void* inc_foo(void* str){
    char* msg; 
    msg = (char *)str;
    if(mutex_enable)
        pthread_mutex_lock(&mutex1);
    
    for(volatile long long int i = 0; i < inc_amnt; i++){
        global_cnt++;
        //printf("%s :%d\n", msg, global_cnt);
       // usleep(10);
    }
    if(mutex_enable)
        pthread_mutex_unlock(&mutex1);
    return NULL;
}
