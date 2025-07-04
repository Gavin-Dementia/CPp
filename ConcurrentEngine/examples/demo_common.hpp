// demo_common.hpp
#ifndef DEMO_COMMON_HPP
#define DEMO_COMMON_HPP

#include <threadPool/threadPool.hpp>
#include <threadPool/scheduler/FIFO_schedule.hpp>
#include <iostream>
#include <chrono>
#include <thread>

inline void runRejectTest(RejectPolicy policy) 
{
    auto scheduler = std::make_unique<FIFOScheduler>();
    scheduler->setRejectPolicy(policy);
    scheduler->setMaxQueueSize(2);

    ThreadPool pool(std::move(scheduler));
    pool.start(1);

    for (int i = 0; i < 10; ++i) 
    {
        try {
            pool.submit([i] {
                std::cout << "[Task " << i << "] Executing...\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }, TaskPriority::MEDIUM);
            std::cout << "[Main] task submit " << i << " success\n";
        } catch (const std::exception& ex) {
            std::cout << "[Main] task submit " << i << " fail: " << ex.what() << "\n";
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    pool.stop();
}

#endif 

