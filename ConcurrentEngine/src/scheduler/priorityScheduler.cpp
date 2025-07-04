#include <threadPool/scheduler/PriorityScheduler.hpp>
#include <stdexcept>

namespace ConcurrentEngine::Scheduler 
{

PriorityScheduler::PriorityScheduler()
    : running_(true),
      rejectPolicy_(RejectPolicy::BLOCK),
      maxQueueSize_(0),
      currentTaskCount_(0)
{
    queues_[TaskPriority::HIGH] = std::queue<Task>{};
    queues_[TaskPriority::MEDIUM] = std::queue<Task>{};
    queues_[TaskPriority::LOW] = std::queue<Task>{};
}

size_t PriorityScheduler::totalQueueSize() const 
{
    return queues_.at(TaskPriority::HIGH).size()
         + queues_.at(TaskPriority::MEDIUM).size()
         + queues_.at(TaskPriority::LOW).size();
}

void PriorityScheduler::addTask(Task task, TaskPriority priority)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (maxQueueSize_ > 0 && currentTaskCount_ >= maxQueueSize_) 
    {
        switch (rejectPolicy_) 
        {
            case RejectPolicy::BLOCK:
                cvFull_.wait(lock, [this] {
                    return currentTaskCount_ < maxQueueSize_;
                });
                break;
            case RejectPolicy::DISCARD:
                std::cout << "[PriorityScheduler] Task discarded (queue full)\n";
                return;
            case RejectPolicy::THROW:
                throw std::runtime_error("[PriorityScheduler] Task rejected (queue full)");
        }
    }

    queues_[priority].push(std::move(task));
    ++currentTaskCount_;
    std::cout << "[PriorityScheduler] Task pushed to priority "
              << static_cast<int>(priority) << "\n";

    lock.unlock();
    cv_.notify_one();
}

void PriorityScheduler::addTask(Task task) 
{  addTask(std::move(task), TaskPriority::MEDIUM); }

Task PriorityScheduler::getTask()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] {
        return totalQueueSize() > 0 || !running_;
    });

    if (!running_ && totalQueueSize() == 0)
        return {};

    for (auto p : {TaskPriority::HIGH, TaskPriority::MEDIUM, TaskPriority::LOW}) 
    {
        if (!queues_[p].empty()) 
        {
            Task task = std::move(queues_[p].front());
            queues_[p].pop();
            --currentTaskCount_;
            cvFull_.notify_one();
            return task;
        }
    }

    return {};
}

void PriorityScheduler::reportStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = totalQueueSize();

    std::cout << "[PriorityScheduler] Queue Status:\n"
              << "  - HIGH   : " << queues_[TaskPriority::HIGH].size() << "\n"
              << "  - MEDIUM : " << queues_[TaskPriority::MEDIUM].size() << "\n"
              << "  - LOW    : " << queues_[TaskPriority::LOW].size() << "\n"
              << "  - TOTAL  : " << total << "\n";
}

void PriorityScheduler::notifyAll()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
}

void PriorityScheduler::setRejectPolicy(RejectPolicy policy)
{
    rejectPolicy_ = policy;
    getRejectPolicy();
}

void PriorityScheduler::getRejectPolicy() const
{
    switch (rejectPolicy_) 
    {
        case RejectPolicy::BLOCK:
            std::cout << "[PriorityScheduler] RejectPolicy BLOCK\n";
            break;
        case RejectPolicy::DISCARD:
            std::cout << "[PriorityScheduler] RejectPolicy DISCARD\n";
            break;
        case RejectPolicy::THROW:
            std::cout << "[PriorityScheduler] RejectPolicy THROW\n";
            break;
    }
}

void PriorityScheduler::setMaxQueueSize(size_t maxSize)
{  maxQueueSize_ = maxSize; }

} // namespace ConcurrentEngine::Scheduler

