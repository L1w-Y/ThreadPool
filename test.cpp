#include <iostream>
#include "threadpool.h"
#include <chrono>
#include <thread>


class mytask : public Task
{
private:
	int begin_;
	int end_;
public:
	Any run() override
	{
		std::cout << "任务开始执行mytask->run(),tid:" << std::this_thread::get_id();
		int sum = 0;
		for (int i =begin_;i<end_;++i)
		{
			sum += i;
		}
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return sum;
	}
	mytask(const int b,const int e):begin_(b),end_(e){}
};

int main() {
	{
		Threadpool pool;
		pool.setMode(PoolMode::MODE_CACHED);
	    pool.start(3);

		std::shared_ptr<Result> r1 = pool.submitTask(std::make_shared<mytask>(1, 100000));
		std::shared_ptr<Result> r2 = pool.submitTask(std::make_shared<mytask>(100001, 200000));
		std::shared_ptr<Result> r3 = pool.submitTask(std::make_shared<mytask>(200001, 300000));
		std::shared_ptr<Result> r4 = pool.submitTask(std::make_shared<mytask>(200001, 300000));
		std::shared_ptr<Result> r5 = pool.submitTask(std::make_shared<mytask>(200001, 300000));
		auto sum = r1->get().cast_<int>();
		auto sum2 = r2->get().cast_<int>();
		auto sum3 = r3->get().cast_<int>();

		std::cout << "sum:" << sum<<std::endl;
		std::cout << "sum2:" << sum2 << std::endl;
		std::cout << "sum3:" << sum3 << std::endl;
	}

	getchar();
}