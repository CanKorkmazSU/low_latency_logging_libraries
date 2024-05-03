#include <gtest/gtest.h>
#include <quill/Quill.h>
#include <iostream>
#include <filesystem>
#include <functional>


/**
 * @brief A generic lambda filter that allows for attaching callbacks and filtering mech. without necessitating for boilerplate code
 * 
*/
class LambdaFilter : public quill::FilterBase
{
public:
  LambdaFilter(const std::function<bool(char const* thread_id, std::chrono::nanoseconds log_message_timestamp,
                              quill::MacroMetadata const& metadata,
                              quill::fmt_buffer_t const& formatted_record)>& filtering_lambda) 
                              : log_filter_(filtering_lambda), quill::FilterBase("LambdaFilter") {};

  QUILL_NODISCARD bool filter(char const* thread_id, std::chrono::nanoseconds log_message_timestamp,
                              quill::MacroMetadata const& metadata,
                              quill::fmt_buffer_t const& formatted_record) noexcept override
  {
    // log only messages that are INFO or above in the file
    return log_filter_(thread_id, log_message_timestamp, metadata, formatted_record);
  }
private:
    std::function<bool(char const* thread_id, std::chrono::nanoseconds log_message_timestamp,
                              quill::MacroMetadata const& metadata,
                              quill::fmt_buffer_t const& formatted_record)> log_filter_;
};

static LambdaFilter UncriticalLevelsFilter{
  std::function<bool(char const* thread_id, std::chrono::nanoseconds log_message_timestamp,
                     quill::MacroMetadata const& metadata, quill::fmt_buffer_t const& formatted_record)>(
    [](char const* thread_id, std::chrono::nanoseconds log_message_timestamp,
       quill::MacroMetadata const& metadata, quill::fmt_buffer_t const& formatted_record) {
        
      return metadata.log_level() >= quill::LogLevel::Info;
    }
  )
};



class QuillTest : public ::testing::Test
{
private:
    quill::Logger *logger_{nullptr};

protected:
    void SetUp() override
    {

        quill::Config cfg;
        //cfg.backend_thread_cpu_affinity = 5;
        //cfg.enable_huge_pages_hot_path = false;
        //cfg.default_queue_capacity = /*default: 131,072*/

        // these two settings are related 
        //cfg.backend_thread_yield 
        //cfg.backend_thread_sleep_duration

        // quill::configure(cfg)

        quill::start(true /*default signal handler, for not to lose messages in case of a crash*/);
        
        //waiting for backend thread to get up
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    void TearDown() override
    {   
        // no need to do anything at the end of program execution, quill will automatically exit 
        // removing loggers for test cases only
        for (auto [key, value] : quill::get_all_loggers())
        {
            //don't delete loger's yourself
            quill::remove_logger(value);
        }
    }

public:
    std::filesystem::path const cur_dir{"/home/ckorkmaz/work/quill_workshop2/src/build/gtest_tests"};

    quill::Logger *get_logger(std::string filename)
    {
        if (logger_)
        {
            return logger_;
        }
        auto handler = quill::file_handler(
                                            filename,
                                            []()
                                            {
                                                quill::FileHandlerConfig cfg;
                                                cfg.set_open_mode('w');
                                                //cfg.queue_capacity((1<<10) * 10);
                                                return cfg;
                                            }()
                                        );

        /*  
        enum Attribute : uint8_t
        {
            Time = 0,
            FileName,
            CallerFunction,
            LogLevel,
            LogLevelId,
            LineNumber,
            Logger,
            FullPath,
            ThreadId,
            ThreadName,
            ProcessId,
            SourceLocation,
            ShortSourceLocation,
            Message,
            CustomTags,
            StructuredKeys,
            ATTR_NR_ITEMS
        };*/
        handler->set_pattern("%(time) %(process_id) %(thread_id) %(logger) LOG_%(log_level) %(message)");

        logger_ = quill::create_logger("default_logger",
                                       quill::file_handler(
                                            filename,
                                            []()
                                            {
                                                quill::FileHandlerConfig cfg;
                                                cfg.set_open_mode('w');
                                                //cfg.queue_capacity((1<<10) * 10);
                                                return cfg;
                                            }()
                                        )
                                    );

        return logger_;
    }
};

// Need to run tests one by one for accurate benchmark metrics since the queue size doesn't reset at each test case, 
// quill::configure() and quill::start() likely takes effect on the first run only

/* TEST_F(QuillTest, WriteTest_Trivial)
{
    auto path = cur_dir / "logs_trivial_quill.log";
    auto *logger = this->get_logger(path.string());

    LOG_INFO(logger, "Info log {}", 1);
    LOG_INFO(logger, "Info log {}", 2);

    LOG_DEBUG(logger, "frontend logger is running on thread: {}", std::this_thread::get_id());
    LOG_INFO(logger, "Logs directed to logs_fnd.log");
    LOG_ERROR(logger, "Logging an error. error code {}", 0xA1);
    LOG_WARNING(logger, "High load on the main thread.");
    LOG_CRITICAL(logger, "A critical error happened. calculated crc's doesn't check.");
} */

/* TEST_F(QuillTest, WriteTest_Runtime_Modifications)
{
    auto path = cur_dir / "logs_runtime_mod_quill.log";
    quill::Logger *logger = this->get_logger(path.string());

    logger->set_log_level(quill::LogLevel::TraceL3);
    ASSERT_TRUE(logger->should_log(quill::LogLevel::TraceL3));
    ASSERT_TRUE(logger->should_log(quill::LogLevel::Critical));

    std::shared_ptr<quill::Handler> stdout_handler = quill::stdout_handler("stdout_handler");
    std::shared_ptr<quill::Handler> file_handler = quill::file_handler(
        cur_dir / "gtest_runnable_quill.log",
        []()
        {
            quill::RotatingFileHandlerConfig cfg;
            cfg.set_rotation_max_file_size((1 << 10) * 2 *//*2mb*//*);
            cfg.set_open_mode('w');
            return cfg;
        }());
    file_handler->set_log_level(quill::LogLevel::Warning);
    quill::Logger *gw_logger = quill::create_logger("gw_logger", {stdout_handler, file_handler});
} */


/* TEST_F(QuillTest, Multiple_Sinks)
{
    quill::fs::path path = cur_dir / "logs_multisink_quill_1.log";
    quill::fs::path path2 = cur_dir/ "logs_multisink_quill_2.log";
    std::shared_ptr<quill::Handler> stdout_handler = quill::stdout_handler("stdout_handler");
    // stdout prints everything
    stdout_handler->set_log_level(quill::LogLevel::TraceL3);
    // first handler only prints severity above warning
    
    std::shared_ptr<quill::Handler> file_handler = quill::file_handler(
                    path,
                        []() -> quill::FileHandlerConfig
                        {
                          quill::RotatingFileHandlerConfig cfg;
                          cfg.set_open_mode('w');
                          return cfg;
                        }());
    file_handler->set_log_level(quill::LogLevel::Warning);

    // second handler directs only those above or eq critical
    quill::FileHandlerConfig cfg_temp;
    cfg_temp.set_open_mode('w');

    auto file_handler_2 = quill::file_handler(
                                    path2, 
                                    std::move(cfg_temp)
                                );
    file_handler_2->set_log_level(quill::LogLevel::Critical);

    auto multiplexing_logger = quill::create_logger("multiplexing_logger", {file_handler, file_handler_2, stdout_handler});
    LOG_TRACE_L3(multiplexing_logger, "This should be printed by only stdout(console)!");
    LOG_INFO(multiplexing_logger, "Also this {}", 1);
    LOG_INFO(multiplexing_logger, "Also this {}", 2);

    LOG_ERROR(multiplexing_logger, "stdout_handler file_handler1 shall print this");
    LOG_WARNING(multiplexing_logger, "stdout_handler file_handler1  shall print this");
    LOG_CRITICAL(multiplexing_logger, "All three prints this");

    //Alternatively set filters for setting callbacks on handlers,
    // as well as managing log levels on a finer granularity level
} */

TEST_F(QuillTest, Callsite_Latency_Async_Hundred_Thousand_Iterations_Dummy)
{
    auto path = cur_dir / "log_async_latency_quill_01.log";
    auto* logger = this->get_logger(path.string());

    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();
    for (int i{}; i < std::pow(10, 5); i++)
    {
        LOG_CRITICAL(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", i);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    std::cout << "Callsite latency for (1/10) million (ms): " << duration.count() << std::endl;
}


/* TEST_F(QuillTest, Callsite_Latency_Async_Million_Iterations_Dummy)
{
    auto path = cur_dir / "log_async_latency_quill_1.log";
    auto* logger = this->get_logger(path.string());

    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();
    for (int i{}; i < std::pow(10, 6); i++)
    {
        LOG_CRITICAL(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", i);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    std::cout << "Callsite latency for million logs (ms): " << duration.count() << std::endl;
} */

/* TEST_F(QuillTest, Callsite_Latency_Async_Ten_Million_Iterations_Dummy)
{
    auto path = cur_dir / "log_async_latency_quill_10.log";
    auto* logger = this->get_logger(path.string());

    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();
    for (int i{}; i < std::pow(10, 7); i++)
    {
        LOG_CRITICAL(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", i);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    std::cout << "Callsite latency for ten million logs (ms): " << duration.count() << std::endl;
} */

/* TEST_F(QuillTest, WriteTest_Million_Iterations_WFlushs_WOPrealloc_Dummy)
{
    std::filesystem::path log_test_logs = cur_dir / "qmillion_iter_woprealloc.log";
    auto *logger = this->get_logger(log_test_logs.string());

    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();
    for (int i{}; i < std::pow(10, 6); i++)
    {
        LOG_CRITICAL(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", i);
    }
    quill::flush();
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    std::cout << "Time to file-log million w flushing (no-prealloc) (ms): " << duration.count() << std::endl;
} */

/* TEST_F(QuillTest, WriteTest_Million_Iterations_WPrealloc)
{
    
    std::filesystem::path log_test_logs = cur_dir / "qmillion_iter_wprealloc.log";
    auto *logger = this->get_logger(log_test_logs.string());
    quill::preallocate();
    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();

    for (int j{};j< std::pow(10, 6); j++) {
    LOG_CRITICAL(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", j);

    }
    quill::flush();
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    std::cout << "Time to write million w flushing (W-prealloc) (ms): " << duration.count() << std::endl;
} */

/* TEST_F(QuillTest, WriteTest_Ten_Million_Iterations_WPrealloc)
{
    
    std::filesystem::path log_test_logs = cur_dir / "qmillion_iter_wprealloc.log";
    auto *logger = this->get_logger(log_test_logs.string());
    quill::preallocate();
    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();

    for (int i{}; i < std::pow(10, 0); i++)
    {
        for (int j{};j< std::pow(10, 7); j++) {
        LOG_CRITICAL(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", i);

        }
        quill::flush();

    }
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    std::cout << "Time to write ten million w flushing (W-prealloc) (ms): " << duration.count() << std::endl;
} */


int main() {
    ::testing::InitGoogleTest();
    [[maybe_unused]] auto i{ RUN_ALL_TESTS() };
}