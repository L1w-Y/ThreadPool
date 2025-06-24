#pragma once

#include "any.h"

class Result;

class Task
{
private:
	std::weak_ptr<Result> result_;
public:
	//用户自定义任务类型，从task基础，重写run方法
	virtual Any run() = 0;
	void exec();
	void setResult(const std::shared_ptr<Result>& res);

	Task() = default;
	~Task() = default;
};
