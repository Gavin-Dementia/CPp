#ifndef CONCURRENTENGINE_SCHEDULER_PRIORITYSCHEDULER_HPP
#define CONCURRENTENGINE_SCHEDULER_PRIORITYSCHEDULER_HPP

#include <threadPool/scheduler/Ischedule.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <iostream>

namespace ConcurrentEngine::Scheduler 
{

enum class TaskPriority { HIGH, MEDIUM, LOW};

class PriorityScheduler : public IScheduler 
{
public:
    PriorityScheduler();

    void addTask(Task task, TaskPriority priority = TaskPriority::MEDIUM);
    void addTask(Task task) override;

    Task getTask() override;
    void reportStatus() override;
    void notifyAll() override;
    void setRejectPolicy(RejectPolicy policy) override;
    void getRejectPolicy() const;
    void setMaxQueueSize(size_t maxSize) override;

    size_t size() const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return totalQueueSize();
    }

    void start() override {};
    void stop() override {};

private:
    size_t totalQueueSize() const;

    std::map<TaskPriority, std::queue<Task>> queues_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable cvFull_;

    bool running_;
    RejectPolicy rejectPolicy_;
    size_t maxQueueSize_;
    size_t currentTaskCount_;
};

} // namespace ConcurrentEngine::Scheduler

#endif // CONCURRENTENGINE_SCHEDULER_PRIORITYSCHEDULER_HPP
