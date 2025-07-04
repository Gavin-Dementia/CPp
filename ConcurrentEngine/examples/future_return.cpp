#include <iostream>
#include <threadPool/threadPool.hpp>
#include <threadPool/scheduler/FIFO_schedule.hpp>
#include <chrono>

int main() 
{
    auto scheduler = std::make_unique<FIFOScheduler>();
    scheduler->setMaxQueueSize(10);
    scheduler->setRejectPolicy(RejectPolicy::BLOCK);

    ThreadPool pool(std::move(scheduler));
    pool.start(2);

    auto fut1 = pool.submit([] { return 42; });
    auto fut2 = pool.submit([](int x, int y) { return x + y; }, 3, 7);

    std::cout << "fut1 = " << fut1.get() << std::endl; // 42
    std::cout << "fut2 = " << fut2.get() << std::endl; // 10

    pool.stop();
    return 0;
}
