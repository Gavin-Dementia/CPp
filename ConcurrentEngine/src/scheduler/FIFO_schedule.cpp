#include <threadPool/scheduler/FIFO_schedule.hpp>

namespace ConcurrentEngine::Scheduler 
{

size_t FIFOScheduler::size() const 
{
    std::lock_guard<std::mutex> lock(mutex_);
    return taskQueue_.size();
}

void FIFOScheduler::setRejectPolicy(RejectPolicy policy) 
{
    rejectPolicy_ = policy;
    getRejectPolicy();
}

void FIFOScheduler::getRejectPolicy() const 
{
    switch (rejectPolicy_) 
    {
        case RejectPolicy::BLOCK:
            std::cout << "[FIFOScheduler] RejectPolicy BLOCK\n";
            break;
        case RejectPolicy::DISCARD:
            std::cout << "[FIFOScheduler] RejectPolicy DISCARD\n";
            break;
        case RejectPolicy::THROW:
            std::cout << "[FIFOScheduler] RejectPolicy THROW\n";
            break;
    }
}

void FIFOScheduler::setMaxQueueSize(size_t maxSize) 
{  maxQueueSize_ = maxSize;  }

void FIFOScheduler::addTask(Task task) 
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (maxQueueSize_ > 0 && taskQueue_.size() >= maxQueueSize_) 
    {
        switch (rejectPolicy_) 
        {
            case RejectPolicy::BLOCK:
                cvFull_.wait(lock, [this] { return taskQueue_.size() < maxQueueSize_; });
                break;
            case RejectPolicy::DISCARD:
                std::cout << "[FIFOScheduler] Task discarded (queue full)\n";
                return;
            case RejectPolicy::THROW:
                throw std::runtime_error("[FIFOScheduler] Task rejected (queue full)");
        }
    }

    taskQueue_.push(std::move(task));
    std::cout << "[FIFOScheduler] Task pushed\n";

    lock.unlock();
    cv_.notify_one();
}

Task FIFOScheduler::getTask() 
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !taskQueue_.empty() || !running_; });

    if (!running_ && taskQueue_.empty())
        return {};

    Task task = std::move(taskQueue_.front());
    taskQueue_.pop();
    cvFull_.notify_one();
    return task;
}

void FIFOScheduler::reportStatus() 
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "[FIFOScheduler] Tasks in queue: " << taskQueue_.size() << std::endl;
}

void FIFOScheduler::notifyAll() 
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
}

} // namespace ConcurrentEngine::Scheduler

