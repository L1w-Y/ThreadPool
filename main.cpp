//
// Created by Administrator on 25-4-10.
//
#include <stdio.h>
#include "threadpool.h"
#include <unistd.h>
void task(void* arg){
    int num = *(int *)arg;
    printf("thread id:%ld,number is %d\n",pthread_self(),num);
    usleep(1000);
}
int main(){

    threadPool* pool= createpool(3,10,100);

    for(int i =0;i<100;++i){
        int* num = (int*)malloc(sizeof(int));
        *num = i + 100;
        threadpooladd(pool,task,num);
    }

    sleep(30);

    threadpooldestroy(pool);
    return 0;
}