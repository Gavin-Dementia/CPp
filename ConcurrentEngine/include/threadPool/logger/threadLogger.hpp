#ifndef THREAD_LOGGER_HPP
#define THREAD_LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>

#ifdef QT_CORE_LIB
#include <QString>
#include <functional>
#endif

#define LOG(X)  ThreadLogger::getInstance().log(X)
#define LOG_INFO(X)  ThreadLogger::getInstance().log(X, LogLevel::INFO)
#define LOG_WARN(X)  ThreadLogger::getInstance().log(X, LogLevel::WARN)
#define LOG_ERROR(X) ThreadLogger::getInstance().log(X, LogLevel::ERROR)
#define LOG_DEBUG(X) ThreadLogger::getInstance().log(X, LogLevel::DEBUG)


enum class LogLevel 
{
    INFO,
    WARN,
    ERROR,
    DEBUG
};

class ThreadLogger 
{
public:
    static ThreadLogger& getInstance();

    void log(const std::string& message, LogLevel level = LogLevel::INFO, int threadID = -1);

    void enableFileLogging(const std::string& filename = "thread.log");
    void disableFileLogging();
    
#ifdef QT_CORE_LIB
    void setGuiLogCallback(std::function<void(const QString&)> callback);
#endif

private:
    ThreadLogger() = default;
    ThreadLogger(const ThreadLogger&) = delete;
    ThreadLogger& operator=(const ThreadLogger&) = delete;

    std::mutex logMutex_;
    bool logToFile_ = false;
    std::ofstream logFile_;

#ifdef QT_CORE_LIB
    std::function<void(const QString&)> guiLogCallback_;
#endif

    std::string getTimestamp();
    std::string getColorPrefix(LogLevel level);
};

#endif // THREAD_LOGGER_HPP
