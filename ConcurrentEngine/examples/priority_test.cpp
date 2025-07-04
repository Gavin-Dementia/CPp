#include <iostream>
#include <thread>
#include <chrono>
#include <threadPool/threadPool.hpp>
#include <threadPool/scheduler/PriorityScheduler.hpp>

void testPriorityScheduler() {
    auto scheduler = std::make_unique<PriorityScheduler>();
    scheduler->setRejectPolicy(RejectPolicy::BLOCK);
    scheduler->setMaxQueueSize(5);

    ThreadPool pool(std::move(scheduler));
    pool.start(2);

    // 提交不同優先級任務
    pool.submit([] {
        std::cout << "[Low Priority] Task running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }, TaskPriority::LOW);

    pool.submit([] {
        std::cout << "[High Priority] Task running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }, TaskPriority::HIGH);

    pool.submit([] {
        std::cout << "[Medium Priority] Task running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }, TaskPriority::MEDIUM);

    pool.submit([] {
        std::cout << "[High Priority 2] Task running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }, TaskPriority::HIGH);

    pool.submit([] {
        std::cout << "[Low Priority 2] Task running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }, TaskPriority::LOW);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    pool.stop();
}

int main() 
{
    testPriorityScheduler();
    return 0;
}
