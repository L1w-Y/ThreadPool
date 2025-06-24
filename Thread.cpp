#include "Thread.h"

#include <thread>

int Thread::generateId_ = 0;

Thread::Thread(const ThreadFunc &func):func_(func),threadId_(generateId_++)
{
}

//Æô¶¯Ïß³Ì
void Thread::start()
{
	std::thread t(func_,threadId_);
	t.detach();
}

Thread::~Thread()
{
	
}

int Thread::getId() const
{
	return threadId_;
}


