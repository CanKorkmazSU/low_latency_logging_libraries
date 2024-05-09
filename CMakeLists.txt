cmake_minimum_required(VERSION 3.0.0)
project(loglibs_demo_spdlog)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/spdlog)
include_directories(${CMAKE_SOURCE_DIR}/src/spdlog/include)

set(TARGET_NAME spdlog_callsite_latency_E5)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE spdlog_callsite_latency_E5.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE spdlog::spdlog)

set(TARGET_NAME spdlog_callsite_latency_E5_four_threads)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE spdlog_callsite_latency_E5_four_threads.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE spdlog::spdlog)

set(TARGET_NAME spdlog_callsite_latency_E6)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE spdlog_callsite_latency_E6.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE spdlog::spdlog)

set(TARGET_NAME spdlog_callsite_latency_E6_four_threads)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE spdlog_callsite_latency_E6_four_threads.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE spdlog::spdlog)



