#include <cstdio>
#include <string>
#include <optional>
#include <iostream>
#include <filesystem>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include "quill/Quill.h"
#include "quill/filters/FilterBase.h"

#define MAX_BACKUP_FILES ((uint)180)


class CustomTimestamp : public quill::TimestampClock
{
public:
  CustomTimestamp() = default;

  /**
   * Required by TimestampClock
   * @return current time now, in nanoseconds since epoch
   */
  uint64_t now() const override { return _ts.load(std::memory_order_relaxed); }

  /**
   * set custom timestamp
   * @param time_since_epoch
   */
  void set_timestamp(std::chrono::seconds time_since_epoch)
  {
    // always convert to nanos
    _ts.store(std::chrono::nanoseconds{time_since_epoch}.count(), std::memory_order_relaxed);
  }

private:
  /**
   * time since epoch - must always be in nanoseconds
   * This class needs to be thread-safe, unless only a single thread in the application calling LOG macros
   * **/
  std::atomic<uint64_t> _ts;
};

class GwFilter : public quill::FilterBase
{
public:
  GwFilter() : quill::FilterBase("GwFilter") {}

  QUILL_NODISCARD bool filter(char const*, std::chrono::nanoseconds,
                              quill::MacroMetadata const& metadata, quill::fmt_buffer_t const&) noexcept override
  {
    if (metadata.log_level() < quill::LogLevel::Warning)
    {
      return true;
    }
    return false;
  }
};

int
main()
{

  std::cout << "Succesffuly run!\n";
  /*std::string engine_log_filename {"logs_engine.log"};
  auto engine_file_handler =
          quill::create_handler<quill::Handler>(engine_log_filename.c_str());

  quill::Logger* engine_logger = quill::create_logger("logger_engine", std::nullopt, std::nullopt);

  std::string bnd_log_filename {"logs_bnd.log"};
  quill::start();*/

  quill::configure(
      []()
      {
        quill::Config cfg;
        return cfg;
      }());

  quill::start();
  
  auto logs_dir = std::filesystem::current_path() / "logs";
  auto const engine_logs_dir    = logs_dir / "engine";
  auto const bnd_logs_dir       = logs_dir / "backend";
  auto const gw_mloop_logs_dir  = logs_dir / "main_looper";

  std::shared_ptr<quill::Handler> gw_daily_rotating =
      quill::rotating_file_handler(gw_mloop_logs_dir / "gw_eng_dly_rt.%N.log",
                                   []()
                                   {
                                     quill::RotatingFileHandlerConfig rfh_cfg;
                                     rfh_cfg.set_rotation_time_daily("00:00");
                                     rfh_cfg.set_rotation_naming_scheme(quill::RotatingFileHandlerConfig::RotationNamingScheme::Date);
                                     // rfh_cfg.set_timezone(quill::Timezone::LocalTime);
                                     rfh_cfg.set_max_backup_files(MAX_BACKUP_FILES);
                                     rfh_cfg.set_rotation_max_file_size(1 << 20 /*1 MB*/);
                                     rfh_cfg.set_open_mode('a');
                                     return rfh_cfg;
                                   }());

  gw_daily_rotating->add_filter(
      std::make_unique<GwFilter>());

  std::cout << "current logs directory is: " << logs_dir << std::endl;
  std::filesystem::path bnd_logs_filename = logs_dir / "../logs_bnd.log";

  quill::Logger *bnd_logger = quill::create_logger(
      "backend_logger",
      quill::file_handler(bnd_logs_filename,
                          []()
                          {
                            quill::RotatingFileHandlerConfig cfg;
                            return cfg;
                          }()));

  bnd_logger->set_log_level(quill::LogLevel::TraceL3);
  bnd_logger->init_backtrace(2u, quill::LogLevel::Critical);

  quill::Logger *fnd_logger = quill::create_logger(
      "frontend_logger",
      quill::file_handler(logs_dir / "../logs_fnd.log",
                          []()
                          {
                            quill::FileHandlerConfig cfg;
                            cfg.set_open_mode('w');
                            return cfg;
                          }()));

  LOG_DEBUG(bnd_logger, "backend logger is running on thread: {}", std::this_thread::get_id());
  LOG_INFO(bnd_logger, "Info log {}", 1);
  LOG_INFO(bnd_logger, "Info log {}", 2);

  LOG_DEBUG(fnd_logger, "frontend logger is running on thread: {}", std::this_thread::get_id());
  LOG_INFO(fnd_logger, "Logs directed to logs_fnd.log");
  LOG_ERROR(fnd_logger, "Logging an error. error code {}", 0xA1);
  LOG_WARNING(fnd_logger, "High load on the main thread.");
  LOG_CRITICAL(fnd_logger, "A critical error happened. calculated crc's doesn't check.");

  quill::Logger *benchmark_logger = quill::create_logger(
      "benchmark_logger",
      quill::file_handler(logs_dir / "benchmark_logs/benchmark.log",
                          []()
                          {
                            quill::FileHandlerConfig cfg;
                            cfg.set_open_mode('w');
                            return cfg;
                          }()));

  std::chrono::high_resolution_clock::time_point start, end;

  start = std::chrono::high_resolution_clock::now();
  for (int i{0}; i < 100; i++)
  {
    LOG_DEBUG(benchmark_logger, " number: {}", i);
  }

  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration1 = end - start;
  std::cout << "Logged 100 logs in " << (double)duration1.count() / std::pow(10, 6) << " ms.\n";

  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++)
  {
    LOG_DEBUG(benchmark_logger, " number: {}", i);
  }

  end = std::chrono::high_resolution_clock::now();
  auto const duration2 = end - start;
  std::cout << "Logged 1000 logs in " << (double)duration2.count() / std::pow(10, 6) << " ms.\n";

  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 10000; i++)
  {
    LOG_DEBUG(benchmark_logger, " number: {}", i);
  }

  end = std::chrono::high_resolution_clock::now();
  quill::flush();
  auto const duration3 = end - start;
  std::cout << "Logged 10000 logs in " << (double)duration3.count() / std::pow(10, 6) << " ms.\n";
  std::cout << "\nMillion logs test:\n";
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < std::pow(10, 7); i++)
  {
    LOG_DEBUG(bnd_logger, " number: {}", i);
  }
  quill::flush();
  end = std::chrono::high_resolution_clock::now();
  auto const duration4 = end - start;
  std::cout << "Logged " << std::pow(10, 7) << "logs in " << (double)duration4.count() / std::pow(10, 6) << " ms.\n";
  return 0;
}