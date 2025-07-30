#include <iostream>
#include <threadPool/threadPool.hpp>

int main() {
    using namespace ConcurrentEngine;
    ThreadPool pool(std::make_unique<Scheduler::FIFOScheduler>());
    pool.start(4);

    // 明確指定名字和優先級，避免衝突
    auto future = pool.submit("MyTask", Scheduler::TaskPriority::MEDIUM, []() -> std::string {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return "Hello from thread pool!";
    });

    std::cout << future.get() << std::endl;
    pool.stop();
    return 0;
}

