#include <iostream>
#include <thread>
#include <chrono>
#include <threadPool/threadPool.hpp>
#include <threadPool/scheduler/DAGschedule.hpp>

// 模擬任務：帶有延遲並印出任務名稱與執行緒ID
ConcurrentEngine::Scheduler::Task makeTask(char name, int delayMs = 100)
{
    return [=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        std::cout << "Task " << name << " completed on thread "
                  << std::this_thread::get_id() << std::endl;
    };
}

int main()
{
    // 自訂 std::terminate 行為，方便偵錯
    std::set_terminate([] {
        std::cerr << "std::terminate() ! probably double delete or bad_function_call. \n";
        std::abort();
    });

    // 建立 DAG 調度器並用它初始化 ThreadPool
    auto dagScheduler = std::make_unique<ConcurrentEngine::Scheduler::DAGScheduler>();
    ConcurrentEngine::ThreadPool pool(std::move(dagScheduler));
    pool.start(4); // 啟動 4 個工作執行緒

    // 建立任務節點
    auto nodeB = std::make_shared<ConcurrentEngine::Scheduler::TaskNode>(makeTask('B'));
    auto nodeC = std::make_shared<ConcurrentEngine::Scheduler::TaskNode>(makeTask('C'));
    auto nodeA = std::make_shared<ConcurrentEngine::Scheduler::TaskNode>(makeTask('A'));

    // 提交 DAG 任務，帶入依賴
    pool.submitDAG(nodeB, {});
    pool.submitDAG(nodeC, {});
    pool.submitDAG(nodeA, {nodeB, nodeC});

    // 等待一段時間讓任務有機會執行完畢
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Finished waiting 3 seconds\n";

    // 停止 ThreadPool
    pool.stop();
    std::cout << "ThreadPool stopped\n";

    return 0;
}
