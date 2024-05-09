#include <iostream>
#include "bench_common.h"
#include "quill/Quill.h"

#define NUM_ITERATIONS 1'000'000

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

        auto on_start = []()
    { quill::preallocate(); };
    auto on_exit = []()
    { /* quill::flush(); */ };
    run_log_benchmark_with_threads(
        NUM_ITERATIONS,
        on_start,
        [&logger](uint64_t i, uint64_t j, double d, std::string const &s)
        {
            LOG_INFO(logger, "Logging int i: {}, int j: {}, double d: {}, string s: {}", i, j, d, s);
        },
        on_exit,
        4);


}

int main(int, char **)
{
    benchmark_process();
    return 0;
}
