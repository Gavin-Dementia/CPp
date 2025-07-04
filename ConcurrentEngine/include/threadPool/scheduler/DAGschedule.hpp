#ifndef CONCURRENTENGINE_SCHEDULER_DAGSCHEDULER_HPP
#define CONCURRENTENGINE_SCHEDULER_DAGSCHEDULER_HPP

#include <threadPool/scheduler/Ischedule.hpp>
#include <threadPool/logger/threadLogger.hpp>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <memory>

namespace ConcurrentEngine::Scheduler
{

using Task = std::function<void()>;

// 單一任務節點，包含依賴計數及依賴節點列表（弱指標避免循環引用）
struct TaskNode 
{
    explicit TaskNode(Task t) : task(std::move(t)) {}
    Task task;
    int dependencyCount = 0;
    std::vector<std::weak_ptr<TaskNode>> dependents;
};

class DAGScheduler : public IScheduler
{
public:
    DAGScheduler() = default;

    void addTask(Task task) override;
    void addTask(std::shared_ptr<TaskNode> node,
                 const std::vector<std::shared_ptr<TaskNode>>& dependencies);

    Task getTask() override;
    void reportStatus() override;
    void notifyAll() override;

    void setRejectPolicy(RejectPolicy) override;
    void setMaxQueueSize(size_t) override;

    size_t size() const override;

    void start() override {}
    void stop() override {}

private:
    void taskCompleted(std::shared_ptr<TaskNode> node);

    std::queue<std::shared_ptr<TaskNode>> readyQueue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;

private:
    bool running_ = true;

};

} // namespace ConcurrentEngine::Scheduler

#endif // CONCURRENTENGINE_SCHEDULER_DAGSCHEDULER_HPP
