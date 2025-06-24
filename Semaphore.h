#pragma once
#include <mutex>

class Semaphore
{
public:
	Semaphore(int limit = 0) :resLimit_(limit){}
	~Semaphore() = default;

	void wait()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		condition_.wait(lock, [&] {return resLimit_ > 0; });
		--resLimit_;
	}

	void post()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		++resLimit_;
		condition_.notify_all();
	}

private:
	int resLimit_;
	std::mutex mtx_;
	std::condition_variable condition_;
};

