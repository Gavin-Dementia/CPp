#include <threadPool/scheduler/DAGschedule.hpp>

namespace ConcurrentEngine::Scheduler
{

void DAGScheduler::addTask(Task task)// 單純不支援以普通 Task 方式加入，這是 DAG 特殊版本
{  std::cout << "[DAGScheduler] addTask(Task) not supported.\n";  }

void DAGScheduler::addTask(std::shared_ptr<TaskNode> node,
                           const std::vector<std::shared_ptr<TaskNode>>& dependencies)
{
    std::unique_lock<std::mutex> lock(mutex_);

    // 設定依賴數量
    node->dependencyCount = static_cast<int>(dependencies.size());

    // 將自己加入各依賴節點的 dependents 清單中
    for (const auto& dep : dependencies)
    {
        if (dep)
            dep->dependents.push_back(node);
    }

    // 若無依賴，直接放入 readyQueue
    if (node->dependencyCount == 0)
    {
        readyQueue_.push(node);
        cv_.notify_one();
    }
}

Task DAGScheduler::getTask()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !readyQueue_.empty() || !running_; });

    if (!running_ && readyQueue_.empty())  return {};
    
    auto node = readyQueue_.front();
    readyQueue_.pop();

    if (!node || !node->task)
    {
        LOG_ERROR("[DAGScheduler] ERROR: null or empty task node in getTask()");
        return {};
    }

    // 回傳一個包裝任務：執行實際任務後通知完成
    return [this, node]() {
        try 
        {  node->task();  } 
        catch (const std::exception& e) 
        {  LOG_ERROR(std::string("[DAGScheduler] Exception in task: ") + e.what());  }
        catch (...) 
        {  LOG_ERROR("[DAGScheduler] Unknown exception in task!");  }
        taskCompleted(node);
    };
}

void DAGScheduler::taskCompleted(std::shared_ptr<TaskNode> node)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& weakDep : node->dependents)
    {
        if (auto dependent = weakDep.lock())
        {
            if (dependent->dependencyCount > 0)
                dependent->dependencyCount--;

            if (dependent->dependencyCount == 0)
            {
                readyQueue_.push(dependent);
                cv_.notify_one();
            }
        }
    }
}

void DAGScheduler::reportStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "[DAGScheduler] Ready queue size: " << readyQueue_.size() << "\n";
}

void DAGScheduler::notifyAll()
{  
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();  
}

void DAGScheduler::setRejectPolicy(RejectPolicy)
{  std::cout << "[DAGScheduler] RejectPolicy not applicable.\n";  }

void DAGScheduler::setMaxQueueSize(size_t)
{  std::cout << "[DAGScheduler] MaxQueueSize not used.\n";  }

size_t DAGScheduler::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return readyQueue_.size();
}

} // namespace ConcurrentEngine::Scheduler

