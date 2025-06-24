#include "Result.h"
#include "task.h"

Result::Result(const std::shared_ptr<Task>& task, const bool isValid):task_(task),isValid_(isValid)
{
}

Any Result::get()
{
	if (!isValid_)
	{
		return "";
	}
	sem_.wait();
	return std::move(any_);
}

void Result::set( Any any)
{
	//´æ´¢task·µ»ØÖµ
	this->any_ = std::move(any);
	sem_.post();
}

