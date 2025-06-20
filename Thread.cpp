#include "Thread.h"

#include <thread>


Thread::Thread(const ThreadFunc &func):func_(func)
{
}

//Æô¶¯Ïß³Ì
void Thread::start()
{
	std::thread t(func_);
	t.detach();
}

Thread::~Thread()
{
	
}

