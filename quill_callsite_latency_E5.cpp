#include <iostream>
#include "bench_common.h"
#include "quill/Quill.h"

#define NUM_ITERATIONS 100'000

void benchmark_process()
{

    quill::Config cfg;
    quill::configure(cfg);
    quill::start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::remove("quill_callsite_latency_E5.log");
    std::shared_ptr<quill::Handler> file_handler =
        quill::file_handler("quill_callsite_latency_E5.log",
                            []()
                            {
                                quill::FileHandlerConfig cfg;
                                cfg.set_open_mode('w');
                                return cfg;
                            }());
    quill::Logger *logger = quill::create_logger("logger", std::move(file_handler));

    std::function<void()> on_start = []()
    { quill::preallocate(); };
    std::function<void()> on_exit = []() { /* quill::flush(); */ };
    std::vector<nanoseconds> latencies(1);
    run_log_benchmark(
        NUM_ITERATIONS,
        on_start,
        [&logger](uint64_t i, uint64_t j, double d, std::string const &s)
        {
            LOG_INFO(logger, "Logging int i: {}, int j: {}, double d: {}, string s: {}", i, j, d, s);
        },
        on_exit,
        1, latencies);
}

int main(int, char **)
{
    benchmark_process();
    return 0;
}
