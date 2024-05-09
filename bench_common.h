#include <functional>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

std::mutex m;

#define LOG_STRING                                                                               \
  "13 Kas 2023 — Class template std::chrono::duration represents a time interval. It consists of a count of ticks of type Rep and a tick period, where the ..."

#define THREAD_LIST_COUNT                                                                          \
  std::vector<int32_t> { 1, 4 }

#define MIN_SLEEP_TIME                                                                            \
  std::size_t { 500 }

#define MAX_SLEEP_TIME                                                                            \
  std::size_t { 3000 }

std::default_random_engine e{std::random_device{}()};
std::uniform_int_distribution<int> dist{MIN_SLEEP_TIME, MAX_SLEEP_TIME};

using namespace std::chrono;
const size_t MESSAGES = 100;


inline void run_log_benchmark(size_t num_iterations,
                              std::function<void()> on_thread_start,
                              std::function<void(uint64_t, uint64_t, double, std::string)> log_func,
                              std::function<void()> on_thread_exit,
                              size_t current_thread_num, std::vector<nanoseconds>& latencies)
{
  on_thread_start();

  log_func(100, 100, 1.0, "");

  // Main Benchmark
  for (int i = 0; i < num_iterations / MESSAGES; ++i)
  {
    double const d = i + (0.1 * i);
    auto start = high_resolution_clock::now();
    for (size_t j = 0; j < MESSAGES; ++j)
    {
      log_func(i, j, d, "");    
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(end - start);
    latencies[current_thread_num-1] += duration;

    std::this_thread::sleep_for(microseconds(dist(e)));
  }
  on_thread_exit();
  std::cout << "Item in latencies as nanoseconds is " << latencies[current_thread_num-1].count() << std::endl;
  std::cout << "Thread " << current_thread_num << " took " << duration_cast<milliseconds>(latencies[current_thread_num-1]).count() << " milliseconds to finish." << std::endl;

}

inline void run_log_benchmark_with_threads(
    size_t num_iterations,
    std::function<void()> on_thread_start,
    std::function<void(uint64_t, uint64_t, double, std::string)> log_func,
    std::function<void()> on_thread_exit,
    size_t num_threads
    )
{
  std::vector<std::thread> threads;
  std::vector<nanoseconds> latencies(num_threads);

  for (int i{1}; i <= num_threads; ++i)
  { 
    std::lock_guard<std::mutex> lock(m); 
    std::thread t(run_log_benchmark, num_iterations, on_thread_start, log_func, on_thread_exit, i, std::ref(latencies));
    threads.push_back(std::move(t));

  }

  for (auto& t : threads)
  {
    t.join();
  }
  std::cout << "All threads finished" << std::endl;
  for (int i{1}; i <= num_threads; ++i)
  {
    std::cout << "Item in latencies as nanoseconds is " << latencies[i-1].count() << std::endl;
    std::cout << "Thread " << i << " took " << duration_cast<milliseconds>(latencies[i-1]).count() << " milliseconds to finish." << std::endl;
  }

}
