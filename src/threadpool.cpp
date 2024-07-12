#include "threadpool.hpp"

namespace rfss {

    ThreadPool::ThreadPool(size_t num_threads) : stop(false){
        for (size_t i = 0; i < num_threads; i++) {
            this->workers.emplace_back( [this] {
                for(;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this]() { return this->stop || !this->tasks.empty(); });
                        if (this->stop || this->tasks.empty())  return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F, typename... Args>
    auto ThreadPool::enqueue(F&& f, Args&&... args) -> void {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)   
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        }
        condition.notify_one();
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex>lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread& worker : workers)
            worker.join();
    }
}