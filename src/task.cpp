#include "slag/task.h"

slag::Task::Task()
    : state_{TaskState::RUNNING}
    , error_{ErrorCode::TRY_AGAIN_LATER}
    , scheduled_{false}
{
}
