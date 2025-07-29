// reject_discard.cpp
#include "demo_common.hpp"

using namespace ConcurrentEngine::Scheduler;
int main() 
{
    runRejectTest(RejectPolicy::DISCARD);
    return 0;
}

