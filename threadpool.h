#pragma once
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <condition_variable>
#include "Thread.h"
class Task
{
public:
	//用户自定义任务类型，从task基础，重新run方法
	virtual void run() = 0;
};


enum class PoolMode
{
	MODE_FIXED, //固定数量
	MODE_CACHED,//动态增长
};

class Threadpool
{
public:
	Threadpool();
	~Threadpool();

	//设置工作模式
	void setMode(PoolMode mode);
	//开起线程池
	void start(int initSize=4);
	//设置任务上限
	void setMaxTaskSizeThreadHold(int threadhold);
	//提交任务
	void submitTask(const std::shared_ptr<Task>& task);
	void threadFunc();
	Threadpool(const Threadpool&) = delete;
	Threadpool operator=(const Threadpool&) = delete;
private:
	std::vector<std::unique_ptr<Thread>> threads_;//线程列表
	size_t initThreadSize_;//初始线程数量

	std::queue<std::shared_ptr<Task>> taskQue_;//任务队列
	std::atomic_uint taskSize_;//任务数量

	int maxTaskSizeThreadHold_;//任务队列数量上限阈值

	std::mutex taskQueMtx_;//保证任务队列的线程安全
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;
	PoolMode mode_;
};

