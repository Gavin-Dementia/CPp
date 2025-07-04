#ifndef CONCURRENTENGINE_SCHEDULER_FIFOSCHEDULER_HPP
#define CONCURRENTENGINE_SCHEDULER_FIFOSCHEDULER_HPP

#include <threadPool/scheduler/Ischedule.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace ConcurrentEngine::Scheduler 
{

class FIFOScheduler : public IScheduler 
{
public:
    FIFOScheduler() = default;

    size_t size() const override;

    void setRejectPolicy(RejectPolicy policy) override;
    void getRejectPolicy() const;

    void setMaxQueueSize(size_t maxSize) override;

    void addTask(Task task) override;

    Task getTask() override;

    void reportStatus() override;

    void notifyAll() override;

    void start() override {}
    void stop() override {}

private:
    std::queue<Task> taskQueue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable cvFull_;

    bool running_ = true;
    RejectPolicy rejectPolicy_ = RejectPolicy::BLOCK;
    size_t maxQueueSize_ = 0;
};

} // namespace ConcurrentEngine::Scheduler

#endif // CONCURRENTENGINE_SCHEDULER_FIFOSCHEDULER_HPP
