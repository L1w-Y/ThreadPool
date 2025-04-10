//
// Created by Administrator on 25-4-10.
//

#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include <pthread.h>
#include <stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#define NUMBER 2
typedef struct Task{

    void(*function)(void* arg);
    void* arg;
}Task;

typedef struct threadPool{
    Task* taskQ;
    int capacity;
    int size;
    int queuefront;
    int queuerear;

    pthread_t managerID;
    pthread_t *threadIDs;
    int minNum;
    int maxNum;
    int busyNum;
    int liveNum;
    int exitNum;
    pthread_mutex_t mutexpool;
    pthread_mutex_t mutexbusy;
    pthread_cond_t notfull;
    pthread_cond_t notempty;
    int shutdown;

}threadPool;

void* worker(void* arg);
void* manager(void* arg);

int threadpoolbusynum(threadPool* pool);
int threadpoolalivenum(threadPool* pool);
int threadpooldestroy(threadPool* pool);

void threadpooladd(threadPool* pool,void(*func)(void*),void *arg);

threadPool* createpool(int min, int max, int queuesize);
void* threadExit(threadPool* pool);

#endif
