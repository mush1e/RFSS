#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>


#ifndef RFSS_THREADPOOL_HPP
#define RFSS_THREADPOOL_HPP

namespace rfss {

    class ThreadPool {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;

    public:
        ThreadPool(size_t num_threads);
        template<typename F, typename... Args>
        void enqueue(F&& f, Args&&... args);
        ~ThreadPool();
    };

}

#endif
