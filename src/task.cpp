#include "slag/task.h"

slag::Task::Task(Executor& executor)
    : executor_{executor}
{
}

slag::Task::~Task() {
    if (is_scheduled()) {
        executor_.cancel(*this);
    }
}

bool slag::Task::is_scheduled() const {
    return static_cast<bool>(scheduled_task_entry_);
}

void slag::Task::schedule(TaskPriority priority) {
    executor_.schedule(*this, priority);
    assert(is_scheduled());
}
