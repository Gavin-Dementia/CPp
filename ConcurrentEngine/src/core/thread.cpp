#include <threadPool/threadPool.hpp>
#include <threadPool/logger/threadLogger.hpp>
#include <iostream>

static std::atomic<int> threadIdGen{0};

Thread::Thread(ThreadFunc func, int id)
    : task_(std::move(func))
    , threadID_(id == -1 ? generateID() : id)
    , hasTask_(false)
    , stopFlag_(false)
{}

Thread::~Thread()
{
    stop();
    if (thread_ && thread_->joinable())
        thread_->join();
}

void Thread::start()
{
    std::cout << "[Thread] thread start()\n";

    if (thread_ && thread_->joinable())
    {
        std::cout << "[Thread] Thread already running\n";
        return; // 避免重複啟動
    }

    // 直接用 task_ 和 threadID_ 啟動執行緒
    thread_ = std::make_unique<std::thread>(task_, threadID_);
}

void Thread::run()
{
    ThreadLogger::getInstance().log("RUN", LogLevel::INFO, threadID_);
    while (true)
    {
        ThreadFunc task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            ThreadLogger::getInstance().log(
                "Waiting for task or stop signal, hasTask = " + std::to_string(hasTask_) + ", stopFlag = " + std::to_string(stopFlag_)
                , LogLevel::INFO, threadID_);
            condVar_.wait(lock, [this]() { return hasTask_ || stopFlag_; });

            if (stopFlag_ && !hasTask_)
                break;

            task = std::move(task_);
            hasTask_ = false;
            ThreadLogger::getInstance().log("Executing task", LogLevel::INFO, threadID_);
        }

        if (task)
            task(threadID_);
            
        if (stopFlag_ && !hasTask_) 
        {
            ThreadLogger::getInstance().log("Stopping thread", LogLevel::INFO, threadID_);
            break;
        }   
    }
}

void Thread::assign(ThreadFunc task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        task_ = std::move(task);
        hasTask_ = true;
    }
    condVar_.notify_one();
}

void Thread::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopFlag_ = true;
    }
    condVar_.notify_one();
}

void Thread::join()
{
    if (thread_ && thread_->joinable())
        thread_->join();
}

bool Thread::joinable() const
{  return thread_ && thread_->joinable();  }

int Thread::getID() const
{
    return threadID_;
}

int Thread::generateID()
{
    return threadIdGen.fetch_add(1);
}

