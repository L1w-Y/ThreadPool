//
// Created by Administrator on 25-4-10.
//

#include "threadpool.h"

void* worker(void* arg){
    threadPool* pool =(threadPool*)arg;

    while(1){
        pthread_mutex_lock(&pool->mutexpool);
        while(pool->size ==0 &&!pool->shutdown){
            pthread_cond_wait(&pool->notempty,&pool->mutexpool);

            if(pool->exitNum >0){
                pool->exitNum--;
                if(pool->liveNum>pool->minNum){
                                    pool->liveNum--;
                pthread_mutex_unlock(&pool->mutexpool);
                threadExit(pool);
                }

            }
        }
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->mutexpool);
            threadExit(pool);
        }

        Task task;
        task.function = pool->taskQ[pool->queuefront].function;
        task.arg = pool->taskQ[pool->queuefront].arg;

        pool->queuefront = (pool->queuefront+1)%pool->capacity;
        pool->size--;
        pthread_cond_signal(&pool->notfull);
        pthread_mutex_unlock(&pool->mutexpool);

        pthread_mutex_lock(&pool->mutexbusy);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexbusy);

        task.function(task.arg);
        free(task.arg);
        task.arg = NULL;
        pthread_mutex_lock(&pool->mutexbusy);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexbusy);
    }

}

int threadpoolbusynum(threadPool* pool){
    pthread_mutex_lock(&pool->mutexbusy);
    int busynum = pool->busyNum;
    pthread_mutex_unlock(&pool->mutexbusy);
    return busynum;
}

int threadpoolalivenum(threadPool* pool){
    pthread_mutex_lock(&pool->mutexpool);
    int livenum = pool->liveNum;
    pthread_mutex_unlock(&pool->mutexpool);
    return livenum;
}

int threadpooldestroy(threadPool* pool){
        if(pool ==NULL){
            return -1;
        }

    pool->shutdown = 1;
    for(int i =0;i<pool->liveNum;i++){
        pthread_cond_signal(&pool->notempty);
    }

    pthread_join(pool->managerID,NULL);

    if(pool->taskQ){
        free(pool->taskQ);
    }
    if(pool->threadIDs){
        free(pool->threadIDs);
    }
    pthread_mutex_destroy(&pool->mutexbusy);
    pthread_mutex_destroy(&pool->mutexpool);
    pthread_cond_destroy(&pool->notempty);
    pthread_cond_destroy(&pool->notfull);

    free(pool);
    pool = NULL;
}

void* manager(void* arg){
    threadPool* pool = (threadPool *)arg;

    while(!pool->shutdown){

        sleep(3);
        pthread_mutex_lock(&pool->mutexpool);
        int queuesize = pool->size;
        int livenum = pool->liveNum;
        pthread_mutex_unlock(&pool->mutexpool);

        pthread_mutex_lock(&pool->mutexbusy);
        int busynum=pool->busyNum;
        pthread_mutex_unlock(&pool->mutexbusy);


        //add thread
        if(queuesize>livenum&&livenum<pool->maxNum){
            pthread_mutex_lock(&pool->mutexpool);
            int count = 0;
            for (int i = 0; i < pool->maxNum && (count < NUMBER) && (pool->liveNum < pool->maxNum) ; ++i) {
                if(pool->threadIDs[i] == 0){
                    pthread_create(&pool->threadIDs[i],NULL,worker,pool);
                    count++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexpool);
        }
        //distory thread
        if(busynum*2<livenum && livenum > pool->minNum){
            pthread_mutex_lock(&pool->mutexpool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexpool);
            for(int i = 0;i<NUMBER;++i){
                pthread_cond_signal(&pool->notempty);
            }

        }
    }

}

void* threadExit(threadPool* pool){
    pthread_t tid = pthread_self();
    for(int i=0;i<pool->maxNum;++i){
        if(pool->threadIDs[i]==tid){
            pool->threadIDs[i]=0;
            printf("threadExit() called,%ld exiting...\n",tid);
            break;
        }
    }
    pthread_exit(NULL);
}

threadPool* createpool(int min, int max, int queuesize){
    threadPool* pool =(threadPool *)malloc(sizeof(threadPool));
    do{
            if(pool ==NULL){
            printf("create failed");
            break;
            }

        pool->threadIDs = (pthread_t *)malloc(sizeof(pthread_t)*max);
        memset(pool->threadIDs,0,sizeof(pthread_t)*max);

        pool->busyNum=0;
        pool->exitNum=0;
        pool->maxNum = max;
        pool->minNum = min;
        pool->liveNum=min;


        if(pthread_mutex_init(&pool->mutexpool,NULL) != 0||
            pthread_mutex_init(&pool->mutexbusy,NULL)!=0||
            pthread_cond_init(&pool->notfull,NULL)!=0||
            pthread_cond_init(&pool->notempty,NULL)!=0){
                printf("mutex or condition init failed");
                break;
            }

        pool->taskQ=(Task*)malloc(sizeof(Task)*queuesize);
        if(pool->taskQ ==NULL){
            break;
        }
        pool->capacity = queuesize;
        pool->size = 0;
        pool->queuefront=0;
        pool->queuerear=0;

        pool->shutdown =0;
        pthread_create(&pool->managerID,NULL,manager,pool);


        for(int i =0;i<min;i++){
            pthread_create(&pool->threadIDs[i],NULL,worker,pool);
        }
        return pool;
    }while(0);

    if(pool->threadIDs) free(pool->threadIDs);
    if (pool->taskQ) free(pool->taskQ);
    if(pool) free(pool);
    return 0;
}

void threadpooladd(threadPool* pool,void(*func)(void*),void *arg){

    pthread_mutex_lock(&pool->mutexpool);

    while(pool->size==pool->capacity && !pool->shutdown){
        pthread_cond_wait(&pool->notfull,&pool->mutexpool);
    }

    if(pool->shutdown){
        pthread_mutex_unlock(&pool->mutexpool);
        return;
    }

    //add task
    pool->taskQ[pool->queuerear].function = func;
    pool->taskQ[pool->queuerear].arg = arg;

    pool->queuerear = (pool->queuerear +1)% pool->capacity;

    pthread_cond_signal(&pool->notempty);

    pthread_mutex_unlock(&pool->mutexpool);

}
