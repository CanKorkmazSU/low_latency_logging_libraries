#include <iostream>
#include "bench_common.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"


#define NUM_ITERATIONS 100'000

void benchmark_process()
{
  spdlog::init_thread_pool(8388608, 1);
      auto sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
        "spdlog_callsite_latency_E5.log");
    auto logger = std::make_shared<spdlog::async_logger>("callsite_logger", sink, spdlog::thread_pool(),
                                                        spdlog::async_overflow_policy::overrun_oldest);
    logger->set_pattern("%T.%F [%t] %s:%# %l     %n - %v");
    std::remove("spdlog_callsite_latency_E5.log");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::function<void()> on_start = []()
    { };
    std::function<void()> on_exit = []() { spdlog::get("callsite_logger")->flush(); };
    std::vector<nanoseconds> latencies(1);
    run_log_benchmark(
        NUM_ITERATIONS,
        on_start,
        [&logger](uint64_t i, uint64_t j, double d, std::string const &s)
        {
            logger->info("Logging int i: {}, int j: {}, double d: {}, string s: {}", i, j, d, s);
        },
        on_exit,
        1, latencies);
}

int main(int, char **)
{
    benchmark_process();
    return 0;
}
