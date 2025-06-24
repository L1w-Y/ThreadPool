#pragma once
#include "any.h"

#include "Semaphore.h"

class Task;

class Result
{
private:
	Any any_;//线程任务返回值
	Semaphore sem_;//线程通信信号量
	std::shared_ptr<Task> task_;//指向该返回值result对应的任务对象，在用户获取返回值之前，保证task对象不被析构
	std::atomic_bool isValid_;
public:
	explicit Result(const std::shared_ptr<Task> &task, const bool isValid = true);
	~Result() = default;

	// 问题一： set方法，获取任务执行完成的返回值
	void set(Any any);
	// get方法，用户调用方法获取task的返回值
	Any get();
};

