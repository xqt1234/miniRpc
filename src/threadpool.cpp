#include "threadpool.h"

ThreadPool::ThreadPool(int baseThreadNum, int maxThreadNum, int maxTaskNum)
    : m_baseThreadNum(baseThreadNum), m_maxThreadNum(maxThreadNum), m_maxTaskNum(maxTaskNum), m_idleThreadNum(m_baseThreadNum)
{
    
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_TaskCv.notify_all();
    m_managerCv.notify_one();
    {
        std::lock_guard<std::mutex> lock(m_threadMapMtx);
        for (auto &[id, threadptr] : m_ThreadMap)
        {
            if (threadptr->joinable())
            {
                threadptr->join();
            }
        }
    }
    if (m_managerThread.joinable())
    {
        m_managerThread.join();
    }
}

void ThreadPool::start()
{
    m_managerThread = std::thread(&ThreadPool::manager, this);
    for (int i = 0; i < m_baseThreadNum; ++i)
    {
        auto newThread = std::make_unique<std::thread>(&ThreadPool::worker, this);
        m_ThreadMap[newThread->get_id()] = std::move(newThread);
        
    }
    m_threadNum+= m_baseThreadNum;
}

void ThreadPool::setNums(int baseThreadNum, int maxThreadNum, int maxTaskNum)
{
    m_baseThreadNum = baseThreadNum;
    m_maxTaskNum = maxThreadNum;
    m_maxTaskNum = maxTaskNum;
}

void ThreadPool::addTask(std::function<void()> f)
{
    std::lock_guard<std::mutex> lock(m_taskMtx);
    if (m_taskque.size() > m_maxTaskNum)
    {
        std::cout << "任务队列已满" << std::endl;
        return;
    }
    else
    {
        m_taskque.emplace(f);
    }
    m_TaskCv.notify_one();
    checkThread();
}

void ThreadPool::checkThread()
{
    int tasknum = 0;
    if (m_threadNum < m_maxThreadNum)
    {
        std::lock_guard<std::mutex> lock(m_taskMtx);
        tasknum = m_taskque.size() - m_idleThreadNum;
        int canCreate = m_maxThreadNum - m_threadNum;
        tasknum = tasknum > canCreate ? canCreate : tasknum;
    }
    if (tasknum > 0)
    {
        std::lock_guard<std::mutex> lock(m_threadMapMtx);
        for (int i = 0; i < tasknum; ++i)
        {
            auto newThread = std::make_unique<std::thread>(&ThreadPool::worker, this);
            m_ThreadMap[newThread->get_id()] = std::move(newThread);
        }
        m_threadNum += tasknum;
        m_idleThreadNum += tasknum;
    }
}

void ThreadPool::manager()
{
    while (!m_stop.load())
    {
        std::unique_lock<std::mutex> lock(m_idsMtx);
        m_managerCv.wait_for(lock, std::chrono::seconds(10), [&]()
                             { return m_ids.size() > 0 || m_stop.load(); });
        if(m_stop)
        {
            break;
        }
        if (m_ids.size() > 0)
        {
            std::lock_guard<std::mutex> lock(m_threadMapMtx);
            for (auto &val : m_ids)
            {
                auto it = m_ThreadMap.find(val);
                if (it != m_ThreadMap.end())
                {
                    if(it->second->joinable())
                    {
                        it->second->join();
                    }
                    m_ThreadMap.erase(it);
                }
            }
        }
    }
}

void ThreadPool::worker()
{
    while (!m_stop.load())
    {
        std::function<void()> task = nullptr;
        {
            std::unique_lock<std::mutex> lock(m_taskMtx);
            if (m_TaskCv.wait_for(lock, std::chrono::seconds(60), [&]()
                                  { return m_taskque.size() > 0 || m_stop.load(); }))
            {
                if (m_stop.load())
                {
                    break;
                }
                if (!m_taskque.empty())
                {
                    task = m_taskque.front();
                    m_taskque.pop();
                    m_idleThreadNum--;
                }
            }
        }
        if (task)
        {
            task();
            m_idleThreadNum++;
        }
        else
        {
            // 40个线程，基础20个，但是，目前15在工作，25个空闲，
            // 相当于20个在工作20个在空闲。此时，空闲工作比例为1.0
            // 假如释放了16个，总线程数量是24个,15个在工作，9个空闲，
            // 此时比例是20比4,比例是0.2，可接受。
            // 基础和最大一样时，即固定模式，比例永远为0，不释放
            int idle = m_idleThreadNum.load();
            int workthread = m_threadNum - idle;
            if (workthread < m_baseThreadNum)
            {
                idle = idle - (m_baseThreadNum - workthread);
                workthread = m_baseThreadNum;
            }
            if (idle * 1.0 / workthread > 0.2)
            {
                {
                    std::lock_guard<std::mutex> lock(m_idsMtx);
                    m_ids.push_back(std::this_thread::get_id());
                    m_idleThreadNum--;
                    m_threadNum--;
                }
            }
        }
    }
}
