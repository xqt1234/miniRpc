#pragma once
#include <queue>
#include <mutex>
#include <optional>
#include <future>
#include <functional>
#include <condition_variable>
#include <iostream>
#include <atomic>
#include <unordered_map>
#include <vector>
class ThreadPool
{
public:

private:
    std::queue<std::function<void()>> m_taskque;
    std::mutex m_taskMtx;
    std::condition_variable m_TaskCv;
    std::thread m_managerThread;
    std::condition_variable m_managerCv;
    int m_baseThreadNum;
    int m_maxThreadNum;
    int m_maxTaskNum;
    std::atomic<int> m_idleThreadNum;
    std::atomic<int> m_threadNum;
    std::atomic<bool> m_stop{true};
    std::unordered_map<std::thread::id,std::unique_ptr<std::thread>> m_ThreadMap;
    std::mutex m_threadMapMtx;
    std::vector<std::thread::id> m_ids;
    std::mutex m_idsMtx;
public:
    ThreadPool(int baseThreadNum = 4,int maxThreadNum = 4,int maxTaskNum = 10);
    ~ThreadPool();
    
    void start();
    void setNums(int baseThreadNum = 4,int maxThreadNum = 4,int maxTaskNum = 10);
    void addTask(std::function<void()> f);
    template<typename Func,typename... Arg>
    auto addTask(Func&& func,Arg... args)->std::optional<std::future<std::invoke_result_t<Func,Arg...>>>
    {
        using RType = std::invoke_result_t<Func,Arg...>;
        auto task = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<Func>(func),std::forward<Arg>(args)...)
        );
        std::future<RType> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(m_taskMtx);
            if(m_taskque.size() > m_maxTaskNum)
            {
                std::cout << "任务队列已满" << std::endl;
                return std::nullopt;
            }else
            {
                m_taskque.emplace([task]{
                    (*task)();
                });
            }
        }
        m_TaskCv.notify_one();
        checkThread();
        return result;
    }
private:
    void checkThread();
    void manager();
    void worker();
};
