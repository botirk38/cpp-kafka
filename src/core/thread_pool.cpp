#include "include/thread_pool.hpp"

ThreadPool::ThreadPool(size_t threads) : stop(false) {
  for (size_t i = 0; i < threads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock lock(queue_mutex);
          condition.wait(lock, [this] { return stop || !tasks.empty(); });
          if (stop && tasks.empty())
            return;
          task = std::move(tasks.front());
          tasks.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for (auto &w : workers) {
    if (w.joinable())
      w.join();
  }
}

void ThreadPool::enqueue(std::function<void()> task) {
  {
    std::unique_lock lock(queue_mutex);
    tasks.push(std::move(task));
  }
  condition.notify_one();
}
