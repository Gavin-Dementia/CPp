#include <threadPool/threadPool.hpp>
#include <threadPool/logger/threadLogger.hpp>
#include <threadPool/core/threadMeta.hpp>
#include <iostream>
#include <chrono>

#if 0
// before
ThreadPool::ThreadPool() : state_(std::make_shared<ThreadPoolState>()) {}

ThreadPool::~ThreadPool() 
{  stop();  }

void ThreadPool::setMode(PoolMode mode) 
{  state_->poolmode = mode;  }

void ThreadPool::setTaskQueueMaxSize(int size) 
{  state_->taskQueueMaxSize = size;  }

void ThreadPool::setMaxThreadCount(size_t maxCount) 
{  state_->maxThreadCount = maxCount;  }

void ThreadPool::createWorkerThread() 
{
    std::cout<<"[ThreadPool] createWorkerThread()\n";

    int tid = state_->threadIDCounter++;
    std::cout << "[ThreadPool] Creating worker thread with ID: " << tid << "\n";

    auto meta = std::make_shared<ThreadMeta>(tid);
    std::cout << "[ThreadPool] Created thread meta for thread ID: " << tid << "\n";
    if(!meta) 
    {
        std::cerr << "[ThreadPool] Failed to create thread meta for thread ID: " << tid << "\n";
        return;
    }

    {
        std::cout<<"[ThreadPool] createWorkerThread() before lock state_->threadMapMutex\n";
        std::lock_guard<std::mutex> lock(state_->threadMapMutex);
        std::cout<<"[ThreadPool] createWorkerThread() getlock state_->threadMapMutex\n";
        threadMetas_[tid] = meta;
        ++state_->curThreadCount;
        ++state_->freeThread;
    }

    // threadFunc 由 ThreadPool 控制，而不是 Thread 裡面自己控制
    meta->thread = std::make_unique<Thread>(
        std::bind(&ThreadPool::threadFunc, this, tid)
    );
    meta->thread->start();

    ThreadLogger::getInstance().log("[ThreadPool] Thread started", tid);
}

void ThreadPool::start() 
{
    std::cout << "[ThreadPool] Starting thread pool...\n";
    state_->isRunning = true;

    switch (state_->poolmode) 
    {
        case PoolMode::MODE_SINGLE:
            std::cout << "[ThreadPool] Starting in MODE_SINGLE.\n";
            state_->initThreadCount= 1; 
        break;
        case PoolMode::MODE_FIXED:
            std::cout << "[ThreadPool] Starting in MODE_FIXED.\n";
            state_->initThreadCount= std::min(size_t(4), state_->maxThreadCount); 
        break;
        case PoolMode::MODE_CACHED:
            std::cout << "[ThreadPool] Starting in MODE_CACHED.\n";
            state_->initThreadCount= 1;
        break;
    }
    
    
    std::cout << "[ThreadPool] start() Initializing " << state_->initThreadCount << " threads.\n";
    
    for (size_t i = 0; i < state_->initThreadCount; ++i)
        createWorkerThread();

    std::cout << "[ThreadPool] start() Started with " << state_->initThreadCount << " threads.\n";
}

void ThreadPool::stop() 
{
    state_->isRunning= false;
    state_->notNull.notify_all();
    state_->notFull.notify_all();

    std::vector<std::shared_ptr<ThreadMeta>> metas;
    {
        std::cout<<"[ThreadPool] stop() before lock state_->threadMapMute \n";
        std::lock_guard<std::mutex> lock(state_->threadMapMutex);
        std::cout << "[ThreadPool] stop() get lock state_->threadMapMutex\n";

        for (auto& [id, meta] : threadMetas_) 
            metas.push_back(meta);

        threadMetas_.clear();
    }

    for (auto& meta : metas) 
    {
        if (meta)
            meta->join(); // 等待線程結束
    }
}

bool ThreadPool::submit(Task task) 
{
    if (!state_->isRunning) 
        return false;

    std::unique_lock<std::mutex> lock(state_->taskQueueMutex);

    if (state_->poolmode == PoolMode::MODE_CACHED &&
        state_->taskCount > state_->freeThread &&
        state_->curThreadCount < state_->maxThreadCount)
    {
        createWorkerThread();
    }

    state_->notFull.wait(lock, [this] {
        return taskQueue_.size() < state_->taskQueueMaxSize || !state_->isRunning;
    });

    if (!state_->isRunning) 
        return false;

    taskQueue_.push(std::move(task));
    ++state_->taskCount;
    lock.unlock();

    state_->notNull.notify_one();
    return true;
}

std::shared_ptr<ThreadMeta> ThreadPool::getThreadMeta(int tid) 
{
    
    std::cout<<"[ThreadPool] getThreadMeta() before lock state_->threadMapMute \n";
    std::lock_guard<std::mutex> lock(state_->threadMapMutex);
    std::cout << "[ThreadPool] getThreadMeta() get lock state_->threadMapMutex\n";
    auto it = threadMetas_.find(tid);

    return it != threadMetas_.end() ? it->second : nullptr;
}

void ThreadPool::threadFunc(int threadid) 
{
    auto meta = getThreadMeta(threadid);
    if (!meta) return;

    while (true) 
    {
        Task task;
        bool timedOut = false;
        {
            std::unique_lock<std::mutex> lock(state_->taskQueueMutex);
            if (state_->poolmode == PoolMode::MODE_CACHED) 
            {
                if (!state_->notNull.wait_for(lock, std::chrono::seconds(15), [this] {
                    return !taskQueue_.empty() || !state_->isRunning;
                })) 
                {
                    timedOut = true;
                }
            } 
            else 
            {
                state_->notNull.wait(lock, [this] {
                    return !taskQueue_.empty() || !state_->isRunning;
                });
            }

            if (!state_->isRunning && taskQueue_.empty()) 
                break;

            if (timedOut && taskQueue_.empty() && 
                state_->curThreadCount > state_->initThreadCount)
            {
                if (meta->shouldRecycle(std::chrono::seconds(15))) 
                    break;
            }


            if (taskQueue_.empty()) continue;

            task = std::move(taskQueue_.front());
            taskQueue_.pop();
            --state_->taskCount;
            --state_->freeThread;
        }

        if (task) 
        {
            meta->markRunning();
            try { task(); } catch (...) {}
            meta->markIdle();
            state_->notFull.notify_one();
        }
    }

    {
        std::cout<<"[ThreadPool] threadfunc() before lock state_->threadMapMute \n";
        std::lock_guard<std::mutex> lock(state_->threadMapMutex);
        std::cout << "[ThreadPool] threadfunc() get lock state_->threadMapMutex\n";
        threadMetas_.erase(threadid);
        meta->markTerminated();
    }
}

#endif