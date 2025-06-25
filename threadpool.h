#pragma once
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <condition_variable>
#include "Thread.h"
#include "any.h"
#include "Result.h"
#include "task.h"
#include <unordered_map>

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
	void start(int initSize = std::thread::hardware_concurrency());
	//设置任务上限
	void setMaxTaskSizeThreadHold(int threadhold);
	//提交任务
	std::shared_ptr<Result> submitTask(const std::shared_ptr<Task>& task);
	void setThreadSizeHold(int size);
	void threadFunc(int threadid);
	Threadpool(const Threadpool&) = delete;
	Threadpool operator=(const Threadpool&) = delete;
private:


	size_t initThreadSize_;//初始线程数量
	std::atomic_int idleThreadSize_;//空闲线程数量
	int threadSizeHold_;//线程数量上限
	std::atomic_int curThreadSize_;//当前线程池中线程的数量

	//std::vector<std::unique_ptr<Thread>> threads_;//线程列表
	std::unordered_map<int, std::unique_ptr<Thread>> threads_;//线程列表
	std::queue<std::shared_ptr<Task>> taskQue_;//任务队列
	std::atomic_uint taskSize_;//任务数量
	int maxTaskSizeThreadHold_;//任务队列数量上限阈值

	std::mutex taskQueMtx_;//保证任务队列的线程安全
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;
	std::condition_variable exitCond_;
	PoolMode mode_;
	std::atomic_bool isPoolRunning_;

	bool checkRunningState() const;
};

