#ifndef CONCURRENTENGINE_SCHEDULER_ISCHEDULER_HPP
#define CONCURRENTENGINE_SCHEDULER_ISCHEDULER_HPP

#include <functional>
#include <cstddef>

namespace ConcurrentEngine::Scheduler 
{

using Task = std::function<void()>;

enum class RejectPolicy 
{
    BLOCK,
    DISCARD,
    THROW
};

class IScheduler 
{
public:
    virtual ~IScheduler() = default;

    virtual void addTask(Task task) = 0;
    virtual Task getTask() = 0;
    virtual void reportStatus() = 0;
    virtual void notifyAll() = 0;
    virtual void setRejectPolicy(RejectPolicy policy) = 0;
    virtual void setMaxQueueSize(size_t maxSize) = 0;
    virtual size_t size() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

} // namespace ConcurrentEngine::Scheduler

#endif // CONCURRENTENGINE_SCHEDULER_ISCHEDULER_HPP

