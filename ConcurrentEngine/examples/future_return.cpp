#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <threadPool/threadPool.hpp>

int main() {
    ConcurrentEngine::ThreadPool pool;
    pool.setMode(ConcurrentEngine::PoolMode::MODE_CACHED);
    pool.start(4);

    // 任務1：沒有參數
    auto fut1 = pool.submit("SimpleReturn", [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 42;
    });

    // 任務2：帶參數的 lambda，使用指定 priority 版本 submit
    auto fut2 = pool.submit(
        ConcurrentEngine::Scheduler::TaskPriority::HIGH,
        [](int x, int y) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return x + y;
        },
        7, 5
    );

    std::cout << "Result from fut1: " << fut1.get() << std::endl;
    std::cout << "Result from fut2: " << fut2.get() << std::endl;

    return 0;
}
