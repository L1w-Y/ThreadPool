#include "threadpool.h"
#include <functional>
#include <iostream>
#include "Thread.h"
constexpr int TASK_MAX_THRESHOLD = 1024;
constexpr int THREAD_MAX_IDLE_TIME = 10;

Threadpool::Threadpool():initThreadSize_(4)
						,taskSize_(0)
						,maxTaskSizeThreadHold_(TASK_MAX_THRESHOLD)
						,mode_(PoolMode::MODE_FIXED)
						,isPoolRunning_(false)
						, idleThreadSize_(0)
						,threadSizeHold_(300)
{
	
}
Threadpool::~Threadpool()
{
	isPoolRunning_ = false;

	std::unique_lock<std::mutex> lock(taskQueMtx_);
	notEmpty_.notify_all();
	exitCond_.wait(lock, [&]
	{
		return threads_.empty();
	});
}

//设置工作模式
void Threadpool::setMode(const PoolMode mode)
{
	if (checkRunningState())
	{
		return;
	}
	mode_ = mode;
}

bool Threadpool::checkRunningState() const
{
	return isPoolRunning_;
}

void Threadpool::setThreadSizeHold(const int size)
{
	if (checkRunningState()) return;
	if (mode_ == PoolMode::MODE_CACHED) threadSizeHold_ = size;
}

//设置任务上限
void Threadpool::setMaxTaskSizeThreadHold(int threadhold)
{
	maxTaskSizeThreadHold_ = threadhold;
}

//提交任务
std::shared_ptr<Result> Threadpool::submitTask(const std::shared_ptr<Task> &task)
{
	//获取锁
	std::unique_lock<std::mutex> lock(taskQueMtx_);
	//线程通信 等待任务队列空余
	if (!notFull_.wait_for(lock,std::chrono::seconds(1), 
		[&]{return taskQue_.size() < static_cast<size_t>(maxTaskSizeThreadHold_); }))
	{
		std::cout << "task queue is full,submit task fail\n";
		return std::make_shared<Result>(task,false);
	}
	//有空间，就放入
	taskQue_.emplace(task);
	auto result = std::make_shared<Result>(task);
	task->setResult(result);
	++taskSize_;
	//通知
	notEmpty_.notify_all();
	if (mode_==PoolMode::MODE_CACHED
		&& taskSize_ >idleThreadSize_
		&&curThreadSize_<threadSizeHold_)
	{
		auto ptr = std::make_unique<Thread>(
			[this](int threadid) { this->threadFunc(threadid); }
		);
		int threadid = ptr->getId();
		threads_.emplace(threadid, std::move(ptr));
		threads_[threadid]->start();
		std::cout << "创建新线程\n";
		++curThreadSize_;
		++idleThreadSize_;
	}
	return result;
}

//开起线程池
void Threadpool::start(int initSize)
{
	isPoolRunning_ = true;
	initThreadSize_ = initSize;
	curThreadSize_ = initSize;
	//创建线程对象
	for (size_t i = 0;i<initThreadSize_;++i)
	{
		auto ptr = std::make_unique<Thread>(
			[this](int threadid) { this->threadFunc(threadid); }
		);
		int threadid = ptr->getId();
		threads_.emplace(threadid, std::move(ptr));
	}

	for (size_t i = 0;i<initThreadSize_;++i)
	{
		threads_[i]->start();
		++idleThreadSize_;
	}
}

//绑定给线程池中的线程作为线程函数，从任务队列中消费任务
void Threadpool::threadFunc(int threadid)
{
	auto lastTime = std::chrono::high_resolution_clock().now();
	for (;;)
	{
		std::shared_ptr<Task> task;
		{
			std::cout << "tid:" << std::this_thread::get_id() << " 尝试获取任务\n";
			//获取锁
			std::unique_lock<std::mutex> lock(taskQueMtx_);

			while(taskQue_.empty())
			{
				if (!isPoolRunning_)
				{
					threads_.erase(threadid);
					--curThreadSize_;
					std::cout << "线程:" << std::this_thread::get_id() << " 被回收\n";
					exitCond_.notify_all();
					return;
				}
				if (mode_ == PoolMode::MODE_CACHED)
				{
					if (std::cv_status::timeout ==
						notEmpty_.wait_for(lock, std::chrono::seconds(1)))
					{
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME
							&& curThreadSize_ > initThreadSize_)
						{
							//回收当前线程
							threads_.erase(threadid);
							--curThreadSize_;
							--idleThreadSize_;
							std::cout << "线程:" << std::this_thread::get_id() << " 被回收\n";
							return;
						}
					}
				}
				else
				{
					//等待notEmpty条件
					notEmpty_.wait(lock);
				}
			}
			--idleThreadSize_;
			//不空就取任务
			task = taskQue_.front();
			taskQue_.pop();
			--taskSize_;
			std::cout << "tid:" << std::this_thread::get_id() << " 获取任务成功\n";
			//如果任务还有，可以通知其他线程执行任务
			if (!taskQue_.empty())
			{
				notEmpty_.notify_all();
			}
			//取出了任务需要通知
			notFull_.notify_all();
		}
		//当前线程执行任务
		if (task != nullptr) {
			task->exec();
			std::cout << "tid:" << std::this_thread::get_id() << "任务执行完成\n";
		}
		lastTime = std::chrono::high_resolution_clock().now();
		++idleThreadSize_;
	}
}



