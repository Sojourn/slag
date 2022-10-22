#include "slag/trace.h"

void magic_trace_stop_indicator() {
    asm volatile("" ::: "memory");
}
