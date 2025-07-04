#ifndef CONCURRENTENGINE_THREADPOOL_HPP
#define CONCURRENTENGINE_THREADPOOL_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <future>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <type_traits>
#include <threadPool/scheduler/FIFO_schedule.hpp>
#include <threadPool/scheduler/DAGschedule.hpp>
#include <threadPool/scheduler/PriorityScheduler.hpp>
#include <threadPool/logger/threadLogger.hpp>
#include <threadPool/core/threadMeta.hpp>

namespace ConcurrentEngine 
{

enum class PoolMode { MODE_SINGLE, MODE_FIXED, MODE_CACHED };

struct ThreadPoolState 
{
    std::atomic<size_t> taskCount{0};
    std::atomic<size_t> curThreadCount{0};
    std::atomic<size_t> freeThread{0};
    std::atomic<int> threadIDCounter{0};
    std::atomic<bool> isRunning{false};

    size_t initThreadCount = 0;
    size_t maxThreadCount = 0;
    size_t taskQueueMaxSize = 0;
    PoolMode poolmode = PoolMode::MODE_FIXED;

    std::mutex taskQueueMutex;
    std::mutex threadMapMutex;
    std::condition_variable notFull;
    std::condition_variable notNull;
};

class ThreadPool 
{
public:
    ThreadPool() = default;

    explicit ThreadPool(std::unique_ptr<Scheduler::IScheduler> scheduler)
        : scheduler_(std::move(scheduler))
        , state_(std::make_shared<ThreadPoolState>()) {}

    void start(int threadCount);
    void stop();

    void reportStatus() 
    {
        std::cout << "[ThreadPool Status]\n"
                  << " - Active Threads: " << state_->curThreadCount << "\n"
                  << " - Free Threads  : " << state_->freeThread << "\n"
                  << " - Total Tasks   : " << state_->taskCount << "\n"
                  << " - Scheduler Queue Size: " << scheduler_->size() << "\n";
    }

    void setScheduler(std::unique_ptr<Scheduler::IScheduler> scheduler) 
    {  scheduler_ = std::move(scheduler);  }

    void submit(Scheduler::Task task);
    bool submit(Scheduler::Task task, Scheduler::TaskPriority priority);

    bool submitDAG(std::shared_ptr<Scheduler::TaskNode> node,
                   const std::vector<std::shared_ptr<Scheduler::TaskNode>>& deps);

    template<typename Func, typename... Args>
    auto submit(const std::string& name, Scheduler::TaskPriority priority, Func&& f, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using ReturnType = std::invoke_result_t<Func, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
        );
        Scheduler::Task wrapper = [task]() { (*task)(); };

        ThreadLogger::getInstance().log("[submit] " + name + " (priority=" + std::to_string(static_cast<int>(priority)) + ")");

        if (!this->submit(wrapper, priority))
            throw std::runtime_error("[ThreadPool::submit] Submit failed");

        return task->get_future();
    }

    template<typename Func, typename... Args>
    auto submit(Scheduler::TaskPriority priority, Func&& f, Args&&... args)
    {
        return submit("UnnamedTask", priority, std::forward<Func>(f), std::forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    auto submit(const std::string& name, Func&& f, Args&&... args)
    {
        return submit(name, Scheduler::TaskPriority::MEDIUM, std::forward<Func>(f), std::forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    auto submit(Func&& f, Args&&... args)
    {
        return submit("UnnamedTask", Scheduler::TaskPriority::MEDIUM, std::forward<Func>(f), std::forward<Args>(args)...);
    }

    template<typename Func>
    auto submitDAG(const std::string& name, Func&& f,
                   const std::vector<std::shared_ptr<Scheduler::TaskNode>>& deps = {})
        -> std::future<std::invoke_result_t<Func>>
    {
        using ReturnType = std::invoke_result_t<Func>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(f));

        auto node = std::make_shared<Scheduler::TaskNode>();
        node->func = [task]() { (*task)(); };

        ThreadLogger::getInstance().log("[submitDAG] " + name);

        if (!this->submitDAG(node, deps))
            throw std::runtime_error("[ThreadPool::submitDAG] Submit DAG task failed");

        return task->get_future();
    }

    template<typename Func>
    auto submitDAG(Func&& f,
                   const std::vector<std::shared_ptr<Scheduler::TaskNode>>& deps = {})
    {
        return submitDAG("UnnamedDAGTask", std::forward<Func>(f), deps);
    }

    size_t getCurThreadCount() const { return state_->curThreadCount; }
    size_t getFreeThreadCount() const { return state_->freeThread; }
    size_t getTaskCount() const { return state_->taskCount; }
    size_t getQueueSize() const { return scheduler_ ? scheduler_->size() : 0; }

    std::shared_ptr<ThreadMeta> getThreadMeta(int tid);
    void workerThreadFunc(int threadId);

private:
    std::unique_ptr<Scheduler::IScheduler> scheduler_;
    std::vector<std::thread> workers_;
    std::shared_ptr<ThreadPoolState> state_;
    std::unordered_map<int, std::shared_ptr<ThreadMeta>> threadMetas_;
};

} // namespace ConcurrentEngine

#endif // CONCURRENTENGINE_THREADPOOL_HPP

