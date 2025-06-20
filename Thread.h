#pragma once
#include <functional>
class Thread
{
public:
	using ThreadFunc = std::function<void()>;
	explicit Thread(const ThreadFunc& func);
	~Thread();
	void start();
private:
	ThreadFunc func_;
};

