#include "../include/thread_pool.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

TEST(ThreadPoolTest, ExecutesTask) {
  std::atomic<int> counter{0};
  ThreadPool pool(2);
  pool.enqueue([&counter] { counter = 1; });
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadPoolTest, ExecutesMultipleTasks) {
  std::atomic<int> counter{0};
  ThreadPool pool(2);
  for (int i = 0; i < 10; i++) {
    pool.enqueue([&counter] { counter++; });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(counter.load(), 10);
}

TEST(ThreadPoolTest, DestructorJoinsWorkers) {
  std::atomic<int> done{0};
  {
    ThreadPool pool(2);
    pool.enqueue([&done] { done++; });
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
  EXPECT_EQ(done.load(), 1);
}
