#include <iostream>
#include "threadpool.h"
#include <chrono>
#include <thread>

class mytask : public Task
{
public:
	void run()
	{
		std::cout << "任务开始执行mytask->run(),tid:" << std::this_thread::get_id();
		std::this_thread::sleep_for(std::chrono::seconds(8));
	}
};

int main()
{
	Threadpool pool;
	pool.start(3);
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());
	pool.submitTask(std::make_shared<mytask>());

}