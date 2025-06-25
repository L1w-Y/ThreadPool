#pragma once
#include <mutex>

class Semaphore
{
public:
	Semaphore(int limit = 0) :resLimit_(limit),isExit_(false){}
	~Semaphore()
	{
		isExit_ = true;
	}

	void wait()
	{
		if (isExit_) return;
		std::unique_lock<std::mutex> lock(mtx_);
		condition_.wait(lock, [&] {return resLimit_ > 0; });
		--resLimit_;
	}

	void post()
	{
		if (isExit_) return;
		std::unique_lock<std::mutex> lock(mtx_);
		++resLimit_;
		condition_.notify_all();
	}

private:
	int resLimit_;
	std::mutex mtx_;
	std::condition_variable condition_;
	std::atomic_bool isExit_;
};

