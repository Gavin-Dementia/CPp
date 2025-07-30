// reject_block.cpp
#include "demo_common.hpp"

using namespace ConcurrentEngine::Scheduler;

int main() 
{
    runRejectTest(RejectPolicy::BLOCK);
    return 0;
}

