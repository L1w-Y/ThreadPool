#include "threadpool.h"
#include <functional>
#include <iostream>
#include "Thread.h"
const int TASK_MAX_THREADHOLD = 3;

Threadpool::Threadpool():initThreadSize_(4)
						,taskSize_(0)
						,maxTaskSizeThreadHold_(TASK_MAX_THREADHOLD)
						,mode_(PoolMode::MODE_FIXED)	
{
	
}
Threadpool::~Threadpool(){}

//设置工作模式
void Threadpool::setMode(const PoolMode mode)
{
	mode_ = mode;
}
//设置任务上限
void Threadpool::setMaxTaskSizeThreadHold(int threadhold)
{
	maxTaskSizeThreadHold_ = threadhold;
}
//提交任务
void Threadpool::submitTask(const std::shared_ptr<Task> &task)
{
	//获取锁
	std::unique_lock<std::mutex> lock(taskQueMtx_);
	//线程通信 等待任务队列空余
	if (!notFull_.wait_for(lock,std::chrono::seconds(1), 
		[&]{return taskQue_.size() < static_cast<size_t>(maxTaskSizeThreadHold_); }))
	{
		std::cout << "task queue is full,submit task fail\n";
		return;
	}
	//有空间，就放入
	taskQue_.emplace(task);
	++taskSize_;
	//通知
	notEmpty_.notify_all();
}
//开起线程池
void Threadpool::start(int initSize)
{
	initThreadSize_ = initSize;
	//创建线程对象
	for (size_t i = 0;i<initThreadSize_;++i)
	{
		threads_.emplace_back(std::make_unique<Thread>([this]() {
			this->threadFunc();
			}));
	}

	for (size_t i = 0;i<initThreadSize_;++i)
	{
		threads_[i]->start();
	}
}


//绑定给线程池中的线程作为线程函数，从任务队列中消费任务
void Threadpool::threadFunc()
{
	for (;;)
	{
		std::shared_ptr<Task> task;
		{
			std::cout << "tid:" << std::this_thread::get_id() << " 尝试获取任务\n";
			//获取锁
			std::unique_lock<std::mutex> lock(taskQueMtx_);
			//等待notEmpty条件
			notEmpty_.wait(lock, [&]{return !taskQue_.empty(); });
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
			task->run();
			std::cout << "tid:" << std::this_thread::get_id() << "任务执行完成\n";
		}
	}

}



