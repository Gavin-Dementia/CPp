#ifndef THREADMETA_HPP
#define THREADMETA_HPP

#include <memory>
#include <chrono>
#include <mutex>
#include <threadPool/logger/threadLogger.hpp>
#include <threadPool/core/thread.hpp>

class Thread;

enum class ThreadState 
{
    Idle,
    Running,
    Terminating,
    Terminated
};

inline const char* toString(ThreadState state) 
{
    switch (state) {
        case ThreadState::Idle: return "Idle";
        case ThreadState::Running: return "Running";
        case ThreadState::Terminating: return "Terminating";
        case ThreadState::Terminated: return "Terminated";
        default: return "Unknown";
    }
}

struct ThreadMeta 
{
    ThreadMeta(int id) : id(id), thread(nullptr)
    {
        ThreadLogger::getInstance().log("[ThreadMeta] Created with ID = " + std::to_string(id), LogLevel::INFO, id);
    }

    ThreadMeta(int id, std::unique_ptr<Thread> thread)
        : id(id)
        , thread(std::move(thread))
        , lastActiveTime(std::chrono::steady_clock::now())
        , state(ThreadState::Idle)
    {
        ThreadLogger::getInstance().log("[ThreadMeta] Initialized with thread. State = Idle", LogLevel::INFO, id);
    }

    int id;
    std::unique_ptr<Thread> thread;
    ThreadState state;
    mutable std::mutex metaMutex;
    std::chrono::steady_clock::time_point lastActiveTime;

    void markIdle() 
    {
        std::lock_guard<std::mutex> lock(metaMutex);
        state = ThreadState::Idle;
        lastActiveTime = std::chrono::steady_clock::now();
        ThreadLogger::getInstance().log("[ThreadMeta] Marked Idle", LogLevel::INFO, id);
    }

    bool isIdle() const 
    {  
        std::lock_guard<std::mutex> lock(metaMutex);
        return state == ThreadState::Idle;  
    }

    void markRunning() 
    {
        std::lock_guard<std::mutex> lock(metaMutex);
        state = ThreadState::Running;
        ThreadLogger::getInstance().log("[ThreadMeta] Marked Running", LogLevel::INFO, id);
    }

    bool shouldRecycle(std::chrono::seconds timeout) const
    {
        std::lock_guard<std::mutex> lock(metaMutex);
        return state == ThreadState::Idle &&
               (std::chrono::steady_clock::now() - lastActiveTime) > timeout;
    }

    void markTerminating() 
    {
        std::lock_guard<std::mutex> lock(metaMutex);
        state= ThreadState::Terminating;
        ThreadLogger::getInstance().log("[ThreadMeta] Marked Terminating", LogLevel::INFO, id);
    }

    void markTerminated() 
    {
        std::lock_guard<std::mutex> lock(metaMutex);
        state= ThreadState::Terminated;
        ThreadLogger::getInstance().log("[ThreadMeta] Marked Terminated", LogLevel::INFO, id);
    }

    void join() 
    {
        if (thread && thread->joinable())
        {
            ThreadLogger::getInstance().log("[ThreadMeta] Joining thread", LogLevel::INFO, id);
            thread->join();
        }
    }
};

#endif // THREAD_META_HPP
