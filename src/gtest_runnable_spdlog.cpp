#include <gtest/gtest.h>
#include <iostream>
#include <initializer_list>
#include <memory>
#include <filesystem>
#include "spdlog/spdlog.h"

int const dummy_long_str = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.";

class SpdlogTest : public ::testing::Test
{
private:
     std::shared_ptr<spdlog::logger> logger_ = nullptr;
     std::shared_ptr<spdlog::logger> sync_logger_ = nullptr;


protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

public:
    static std::filesystem::path const cur_dir{"/home/ckorkmaz/work/quill_workshop2/src/build/gtest_tests"};

     inline std::shared_ptr<spdlog::logger> get_async_logger_spd(std::string filename = (cur_dir / "default_test_logs.log").string())
    {
        if(logger_.get()) return logger_;

        logger_ = std::make_shared<spdlog::async_logger>("gtest_runnable_spdlog_aysnc", filename);
        logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
        return logger_;
    }

     inline std::shared_ptr<spdlog::logger> get_sync_logger_spd(std::string filename = (cur_dir / "default_test_logs.log").string())
    {
        if (sync_logger_) {
            delete sync_logger_;
            sync_logger_ = nullptr;
        }
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
        sync_logger_ = std::make_shared<spdlog::logger>("gtest_runnable_spdlog_sync", sink);
        return sync_logger_;
    }
};

TEST_F(SpdlogTest, WriteTest_Trivial)
{
    auto path = SpdlogTest::cur_dir / "logs_trivial_spdlog.log";
    auto logger = this->get_sync_logger_spd(path.string());

    spdlog::info(logger, "Info log {}", 1);
    spdlog::info(logger, "Info log {}", 2);

    spdlog::debug(logger, "frontend logger is running on thread: {}", std::this_thread::get_id());
    spdlog::info(logger, "Logs directed to logs_fnd.log");
    spdlog::error(logger, "Logging an error. error code {}", 0xA1);
    spdlog::warning(logger, "High load on the main thread.");
    spdlog::critical(logger, "A critical error happened. calculated crc's doesn't check.");
}

TEST_F(SpdlogTest, WriteTest_Runtime_Modifications)
{
    auto path = cur_dir / "logs_runtime_mod_spdlog.log";
    auto logger = this->get_async_logger_spd(path.string());

    logger->set_level(spdlog::level::from_str("DEBUG"));

    ASSERT_EQ(logger->level(), spdlog::level::DEBUG);
}

TEST_F(SpdlogTest, Multiple_Sinks)
{
    auto path = cur_dir / "logs_runtime_mod.log";
    auto logger = this->get_async_logger_spd(path.string());

    auto stdout_logger = spdlog::stdout_logger_mt("stdout_logger");
    auto file_logger = spdlog::rotating_logger_mt("file_logger", "gtest_runnable_spdlog.log", (1 << 10) * 5, 0, true);
    file_logger->set_level(spdlog::level::warn);

    spdlog::logger multisink_logger("multisink_logger");
    multisink_logger.sinks().push_back(stdout_logger);
    multisink_logger.sinks().push_back(file_logger);

    spdlog::debug(multisink_logger, "a spdlog multi sink loggin example");
}

TEST_F(SpdlogTest, Callsite_Latency_Async_Million_Iterations_Dummy)
{
    std::filesystem::path log_test_logs = SpdlogTest::cur_dir / "million_test_logs_sync_spdlog.log";

    auto logger = this->get_async_logger_spd();
    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();
    //calls same function 1'000'000 times without flush'ing at the end, dummy scenario
    for (int i{}; i < std::pow(10, 6); i++)
    {
        spdlog::debug(logger, "Log that is longer than length 35 for manevouring short string optimizaiton, iter: {}", i);
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << "Callsite latency w async million log writes (ms): " << duration.count() << std::endl;
}

TEST_F(SpdlogTest, Callsite_Latency_Async_Million_Iterations_Scenario)
{
    std::filesystem::path log_test_logs = SpdlogTest::cur_dir / "million_test_logs_sync_spdlog.log";

    auto logger = this->get_async_logger_spd();
     
    auto log_func_int = [&logger](uint64_t i, uint64_t j, double d) {
        spdlog::critical(logger, "Logging int: {}, int: {}, double: {}", i, j, d);
    };
    auto log_func_str = [&logger](uint64_t i, uint64_t j) {
        spdlog::critical(logger, "Logging int: {}, int: {}, string: {}", i, j, dummy_long_str);
    };
    srand(time(0));
    auto start = std::chrono::high_resolution_clock::now();
    //calls same function 1'000'000 times without flush'ing at the end, more realistic scenaroi
    for (int i{}; i < std::pow(10, 6); i++)
    {
        if(rand() % 2)
            log_func_int(i, i+1, i +1.1 );
        else 
            log_func_str(i, i+1);
        if (i != 0 && i % 500) {
            std::this_thread::sleep_for(std::chrono::duration<std::chrono::milliseconds>(2));
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    duration -= std::chrono::duration_cast<std::chrono::milliseconds>((std::pow(10,6) / 500 ) * 2);
    std::cout << "Callsite latency million log writes (ms): " << duration.count() << std::endl;
}


TEST_F(SpdlogTest, Sync_Million_Iterations_TotalDuration_Dummy)
{
    std::filesystem::path log_test_logs = SpdlogTest::cur_dir / "million_test_logs_spdlog.log";

    auto logger = this->get_sync_logger_spd();
    std::chrono::high_resolution_clock::time_point start, end;
    start = std::chrono::high_resolution_clock::now();
    for (int i{}; i < std::pow(10, 6); i++)
    {
        spdlog::debug(logger, "Log that is longer than length 35 for bypassing short string optimizaiton, iter: {}", i);
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << "Time to write million logs (ms): " << duration.count() << std::endl;
}


int main() {
    ::testing::InitGoogleTest();
    [[maybe_unused]] auto i{ RUN_ALL_TESTS() };
}