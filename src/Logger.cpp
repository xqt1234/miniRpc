#include "Logger.h"
#include <filesystem>
const int maxSize = 100 * 1024 * 1024;
Logger &Logger::getInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    
    //setLogFile("muduo.log");
    setLogFile(createDefaultFileName());
    m_workThread = std::thread([this]
                               { this->processBuffers(); });
}

Logger::~Logger()
{
    shutdown();
}

void Logger::flush()
{
    {
        std::unique_lock<std::mutex> lock(m_buffMtx);
        swapBuffers();
    }
    m_cv.notify_one();
}

void Logger::shutdown()
{
    flush();
    bool expected = false;
    if (!m_exit.compare_exchange_strong(expected, true))
    {
        return;
    }
    //设置了m_exit之后，没有被唤醒，是不行的。
    m_cv.notify_all();
    if (m_workThread.joinable())
    {
        m_workThread.join();
    }
    if (m_ofs.is_open())
    {
        m_ofs.close();
    }
}

void Logger::setLogLevel(LogLevel level)
{
    m_level = level;
}

void Logger::swapBuffers()
{
    if (!m_activeBuffers.empty())
    {
        m_activeBuffers.swap(m_readBuffers);
    }
}

void Logger::setLogFile(const std::string &filename)
{
    m_filename = filename;
    if (m_ofs.is_open())
    {
        m_ofs.close();
    }
    m_ofs.open(m_filename, std::ios::out | std::ios::app);
    if (!m_ofs.is_open())
    {
        throw "cannot open Log file";
    }
}

void Logger::processBuffers()
{
    std::vector<std::string> localBuffers;
    const auto flush_interval = std::chrono::seconds(1);
    while (!m_exit)
    {
        {
            std::unique_lock<std::mutex> lock(m_buffMtx);
            m_cv.wait_for(lock,flush_interval, [this]
                      { return !m_readBuffers.empty() || m_exit; });
            if(m_readBuffers.empty() && !m_activeBuffers.empty())
            {
                swapBuffers();
            }
            localBuffers.swap(m_readBuffers);
        }
        
        if (!localBuffers.empty())
        {
            for (const auto &s : localBuffers)
            {
                m_ofs << s << '\n';
            }
            m_ofs.flush();
            localBuffers.clear();
            checkfile();
        }
    }
}

std::string Logger::createDefaultFileName()
{
    std::filesystem::path logdir = "log";
    if(!std::filesystem::exists(logdir))
    {
        std::filesystem::create_directories(logdir);
    }
    auto now = std::chrono::system_clock::now();
    auto local = floor<std::chrono::milliseconds>(now);
    auto local_time = std::chrono::zoned_time{std::chrono::current_zone(), local};
    std::string filename = std::format("{:%Y%m%d%H%M%S}.log", local_time);
    return (logdir / filename).string();
}

void Logger::checkfile()
{
    if(maxSize < m_ofs.tellp())
    {
        m_ofs.close();
        setLogFile(createDefaultFileName());
    }
}

const char *Logger::logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::buildRecord(LogLevel level, const std::source_location &loc, const std::string &msg)
{
    auto now = std::chrono::system_clock::now();
    auto local = floor<std::chrono::milliseconds>(now);
    auto local_time = std::chrono::zoned_time{std::chrono::current_zone(), local};
    
    
    return std::format("[{:%F %T}][{}][{}:{}] {}", local_time, logLevelToString(level), loc.file_name(), loc.line(), msg);
}
