#include "Task.h"
#include "Result.h" 
#include <iostream>
void Task::exec()
{
    if (const auto res = result_.lock())
    {
        res->set(run());

    }
    else
    {
        std::cout << "»ñÈ¡resultÖ¸ÕëÊ§°Ü\n";
    }
}

void Task::setResult(const std::shared_ptr<Result>& res)
{
    result_ = res;
}
