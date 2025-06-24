#pragma once
#include <functional>
class Thread
{
public:
	using ThreadFunc = std::function<void(int)>;
	explicit Thread(const ThreadFunc& func);
	~Thread();
	void start();
	int getId()const;
private:
	ThreadFunc func_;
	static int generateId_;
	int threadId_;
};

