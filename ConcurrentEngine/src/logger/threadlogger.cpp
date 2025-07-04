#include <threadPool/logger/threadLogger.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>

ThreadLogger& ThreadLogger::getInstance() 
{
    static ThreadLogger instance;
    return instance;
}

void ThreadLogger::log(const std::string& message, LogLevel level, int threadID) 
{
    static thread_local bool reentry = false;
    if (reentry) return;  // 防止遞迴 log 導致 terminate
    reentry = true;

    try 
    {
        std::lock_guard<std::mutex> lock(logMutex_);

        std::string timestamp = getTimestamp();
        std::string levelStr;
        std::string color = getColorPrefix(level);

        switch (level) 
        {
            case LogLevel::INFO:  levelStr = "INFO";  break;
            case LogLevel::WARN:  levelStr = "WARN";  break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
        }

        std::ostringstream finalMsg;
        finalMsg << "[" << timestamp << "] [" << levelStr << "] ";
        if (threadID == -1)
            finalMsg <<"";
        else
            finalMsg << "Thread " << threadID << ": ";
        finalMsg << message;

        // 安全輸出到 console
        std::fputs((color + finalMsg.str() + "\033[0m\n").c_str(), stdout);
        std::fflush(stdout); // 確保同步輸出

        // 寫入 log 檔案（若啟用）
        if (logToFile_ && logFile_.is_open()) 
        {
            logFile_ << finalMsg.str() << std::endl;
        }

#ifdef QT_CORE_LIB
        if (guiLogCallback_) 
        {
            QString qmsg = QString::fromStdString(finalMsg.str());
            guiLogCallback_(qmsg);
        }
#endif

    } catch (...) 
    {  std::fputs("[ThreadLogger] Logging failed due to exception.\n", stderr);  }

    reentry = false;
}

void ThreadLogger::enableFileLogging(const std::string& filename) 
{
    std::lock_guard<std::mutex> lock(logMutex_);
    logFile_.open(filename, std::ios::out | std::ios::app);
    logToFile_ = logFile_.is_open();
}

void ThreadLogger::disableFileLogging() 
{
    std::lock_guard<std::mutex> lock(logMutex_);
    if (logFile_.is_open())
        logFile_.close();
    logToFile_ = false;
}

std::string ThreadLogger::getTimestamp() 
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string ThreadLogger::getColorPrefix(LogLevel level) 
{
    switch (level) 
    {
        case LogLevel::INFO:  return "\033[32m"; // green
        case LogLevel::WARN:  return "\033[33m"; // yellow
        case LogLevel::ERROR: return "\033[31m"; // red
        case LogLevel::DEBUG: return "\033[36m"; // cyan
        default:              return "";
    }
}

#ifdef QT_CORE_LIB
void ThreadLogger::setGuiLogCallback(std::function<void(const QString&)> callback) 
{
    std::lock_guard<std::mutex> lock(logMutex_);
    guiLogCallback_ = std::move(callback);
}
#endif

