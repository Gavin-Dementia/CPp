#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <unordered_map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <future>
#include <threadpool/core/thread.hpp>
#include <threadPool/core/threadMeta.hpp>


#if 0
// before
class ThreadPool {
public:
    using Task = std::function<void()>;

    ThreadPool();
    ~ThreadPool();

    std::shared_ptr<ThreadMeta> getThreadMeta(int tid);
    void setMode(PoolMode mode);
    void setTaskQueueMaxSize(int size);
    void setMaxThreadCount(size_t maxCount);
    void createWorkerThread();
    void start();
    void stop();

    bool submit(Task task);

    template<typename Func, typename... Args>
    auto submit(Func&& f, Args&& ... args) 
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using ReturnType = decltype(f(args...));

        auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)... )
        );

        Task wrapperYask= [taskPtr ]{ (*taskPtr )(); };

        bool success= this->submit(std::move(wrapperYask));
        if (!success)
            throw std::runtime_error("ThreadPool is not running or queue full");

        return taskPtr->get_future();
    }

private:
    void threadFunc(int threadid);

private:
    std::shared_ptr<ThreadPoolState> state_;
    std::unordered_map<int, std::shared_ptr<ThreadMeta>> threadMetas_;
    std::queue<Task> taskQueue_;
};
#endif


#endif

