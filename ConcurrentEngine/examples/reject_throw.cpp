// reject_throw.cpp
#include "demo_common.hpp"

using namespace ConcurrentEngine::Scheduler;
int main() 
{
    runRejectTest(RejectPolicy::THROW);
    return 0;
}