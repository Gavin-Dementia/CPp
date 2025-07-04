#ifndef THREAD_HPP
#define THREAD_HPP

#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>

class Thread 
{
public:
    using ThreadFunc = std::function<void(int)>;

    Thread() : threadID_(-1), stopFlag_(false), hasTask_(false) {}

    explicit Thread(ThreadFunc func, int id = -1);
    ~Thread();

    void start();  // 啟動執行緒
    void join();   // 等待結束
    bool joinable() const; // 檢查是否可 join

    int getID() const;
    void assign(ThreadFunc task); // 分配任務給執行緒
    void stop(); // 停止執行緒循環

private:
    void run();  // 執行緒主迴圈

private:
    static int generateID(); // 自動遞增的 thread id

    int threadID_;
    ThreadFunc task_;
    std::unique_ptr<std::thread> thread_;

    std::mutex mutex_;
    std::condition_variable condVar_;

    bool hasTask_;
    bool stopFlag_;
};

#endif // THREAD_POOL_THREAD_HPP

