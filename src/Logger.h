#include <iostream>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include <memory>
#include <vector>
#include <atomic>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string_view>
#include <format>
#include <thread>
#include <source_location>
/**
 * @brief 主要思路：使用两个buffer，一个前台一个后台，定时定量写入磁盘。
 * 通过swap，直接换数据，效率高。
 * 
 */
#define LOG_DEBUG(fmt, ...) \
    Logger::getInstance().log(LogLevel::DEBUG, std::source_location::current(), fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    Logger::getInstance().log(LogLevel::INFO, std::source_location::current(), fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    Logger::getInstance().log(LogLevel::WARN, std::source_location::current(), fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
    Logger::getInstance().log(LogLevel::ERROR, std::source_location::current(), fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) \
    Logger::getInstance().log(LogLevel::FATAL, std::source_location::current(), fmt, ##__VA_ARGS__)
enum class LogLevel
{
    INFO,
    DEBUG,
    WARN,
    ERROR,
    FATAL,
};

class Logger
{
private:
    std::vector<std::string> m_activeBuffers;
    std::vector<std::string> m_readBuffers;
    std::mutex m_buffMtx;
    std::condition_variable m_cv;
    std::string m_filename;
    std::ofstream m_ofs;
    std::atomic<bool> m_exit{false};
    std::atomic<LogLevel> m_level{LogLevel::DEBUG};
    static constexpr size_t BUFFER_LIMIT{10000};
    std::thread m_workThread;

public:
    static Logger &getInstance();
    template <typename... Args>
    void log(LogLevel level, const std::source_location &loc, std::string_view fmt, Args... args)
    {
        //return;
        if (m_exit.load(std::memory_order_acquire) || level < m_level)
        {
            return;
        }
        std::string msg;
        try
        {
            msg = std::vformat(fmt, std::make_format_args(args...));
        }
        catch (const std::exception &e)
        {
            msg = "Format Error:" + std::string(fmt);
        }
        std::string recode = buildRecord(level, loc, msg);
        {
            std::unique_lock<std::mutex> lock(m_buffMtx);
            m_activeBuffers.push_back(recode);
            if (m_activeBuffers.size() >= BUFFER_LIMIT)
            {
                swapBuffers();
                m_cv.notify_one();
            }
        }
        if (level == LogLevel::FATAL)
        {
            shutdown();
            std::abort();
            //std::terminate();
        }
    }
    void flush();
    void shutdown();
    void setLogLevel(LogLevel level);
private:
    Logger();
    ~Logger();
    
    void swapBuffers();
    void setLogFile(const std::string &filename);
    void processBuffers();
    std::string createDefaultFileName();
    void checkfile();
    const char* logLevelToString(LogLevel level);
    std::string buildRecord(LogLevel level,const std::source_location &loc,const std::string &msg);
};