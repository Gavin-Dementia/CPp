// namespace ConcurrentEngine
#include <threadPool/threadPool.hpp>

namespace ConcurrentEngine 
{

// 啟動 ThreadPool，建立指定數量的工作執行緒
void ThreadPool::start(int threadCount)
{
    if (state_ && state_->isRunning) return;

    state_ = std::make_shared<ThreadPoolState>();
    ThreadLogger::getInstance().log("[ThreadPool] Starting with " + std::to_string(threadCount) + " threads.");

    for (int i = 0; i < threadCount; ++i)
    {
        int threadId = state_->threadIDCounter++;
        auto meta = std::make_shared<ThreadMeta>(threadId);
        {
            std::lock_guard<std::mutex> lock(state_->threadMapMutex);
            threadMetas_[threadId] = meta;
        }

        workers_.emplace_back(&ThreadPool::workerThreadFunc, this, threadId);
    }

    state_->isRunning = true;
    ThreadLogger::getInstance().log("[ThreadPool] State set to running.");

}

// 停止 ThreadPool，通知所有工作執行緒結束並等待它們 join
void ThreadPool::stop()
{
    if (!state_->isRunning) return;

    state_->isRunning = false;

    scheduler_->notifyAll(); // 通知 Scheduler 停止，喚醒所有阻塞執行緒
    ThreadLogger::getInstance().log("[ThreadPool] Stopping...");

    for (auto& t : workers_)
    {
        if (t.joinable())
            t.join();
    }

    ThreadLogger::getInstance().log("[ThreadPool] All worker threads joined.");
}

// 依 threadId 取得 ThreadMeta（紀錄該執行緒狀態）
std::shared_ptr<ThreadMeta> ThreadPool::getThreadMeta(int tid)
{
    std::lock_guard<std::mutex> lock(state_->threadMapMutex);
    auto it = threadMetas_.find(tid);
    if (it != threadMetas_.end())
            return it->second;
    else
        return nullptr;
}

// 支援以 threadId 追蹤執行緒狀態的工作函式
void ThreadPool::workerThreadFunc(int threadId)
{
    auto meta = getThreadMeta(threadId);
    if (!meta) 
    {
        ThreadLogger::getInstance().log("[Worker] Error: No ThreadMeta found", LogLevel::INFO, threadId);
        return;
    }

    ThreadLogger::getInstance().log("[Worker] Thread started", LogLevel::INFO, threadId);

    while (state_->isRunning)
    {
        ConcurrentEngine::Scheduler::Task task = scheduler_->getTask(); // 阻塞等待任務
        if (!task && !state_->isRunning) break;

        meta->markRunning();
        ThreadLogger::getInstance().log("[Worker] Task started", LogLevel::INFO, threadId);

        try 
        {  task();  } 
        catch (const std::exception& e) 
        {
            ThreadLogger::getInstance().log(std::string("[Worker] Task exception: ") + e.what(), LogLevel::INFO, threadId);
        }

        ThreadLogger::getInstance().log("[Worker] Task finished", LogLevel::INFO, threadId);
        meta->markIdle();
    }

    meta->markTerminating();
    ThreadLogger::getInstance().log("[Worker] Thread exiting", LogLevel::INFO, threadId);
    meta->markTerminated();
}

// 提交普通任務，帶優先級的版本
bool ThreadPool::submit(Scheduler::Task task, Scheduler::TaskPriority priority)
{
    if (!scheduler_) 
    {
        ThreadLogger::getInstance().log("[ThreadPool] Submit failed: No scheduler.");
        return false;
    }

    if (!state_ || !state_->isRunning)
    {
        ThreadLogger::getInstance().log("[ThreadPool] Submit failed: Not running.");
        return false;
    }

    ThreadLogger::getInstance().log("[ThreadPool] Task submitted with priority " + std::to_string(static_cast<int>(priority)));

    // DAG 調度器不接受普通任務直接提交
    if (dynamic_cast<Scheduler::DAGScheduler*>(scheduler_.get())) 
    {
        ThreadLogger::getInstance().log("[ThreadPool] DAG Scheduler does not accept plain Task submit.");
        return false;
    }

    if (auto* pri = dynamic_cast<Scheduler::PriorityScheduler*>(scheduler_.get()))
        pri->addTask(std::move(task), priority);   
    else
        scheduler_->addTask(std::move(task));
    

    return true;
}

// 專用 DAG 任務提交（包含依賴）
bool ThreadPool::submitDAG(std::shared_ptr<Scheduler::TaskNode> node,
                   const std::vector<std::shared_ptr<Scheduler::TaskNode>>& deps)
{
    if (!scheduler_ || !state_ || !state_->isRunning) return false;

    auto* dag = dynamic_cast<Scheduler::DAGScheduler*>(scheduler_.get());
    LOG_INFO("==== submitDAG ====");

    if (!dag) 
    {
        LOG_ERROR("[ThreadPool] Current scheduler is not DAG.");
        return false;
    }

    if (!node || !node->task) 
    {
        LOG_ERROR("[ThreadPool] ERROR: submitDAG received invalid task.");
        return false;
    }

    dag->addTask(node, deps);
    ThreadLogger::getInstance().log("[ThreadPool] DAG task submitted.");
    return true;
}

} // namespace ConcurrentEngine

