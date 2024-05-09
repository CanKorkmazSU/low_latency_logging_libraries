cmake_minimum_required(VERSION 3.0.0)
project(loglibs_demo_quill VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(CMAKE_BUILD_TYPE Release)

    add_subdirectory(${CMAKE_SOURCE_DIR}/src/quill)
    include_directories(${CMAKE_SOURCE_DIR}/src/quill/quill)
    include_directories(${CMAKE_SOURCE_DIR}/src/quill/quill/include)

include_directories(${CMAKE_SOURCE_DIR})

set(TARGET_NAME quill_callsite_latency_E5)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE quill_callsite_latency_E5.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE quill)

set(TARGET_NAME quill_callsite_latency_E5_four_threads)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE quill_callsite_latency_E5_four_threads.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE quill)

set(TARGET_NAME quill_callsite_latency_E6)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE quill_callsite_latency_E6.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE quill)

set(TARGET_NAME quill_callsite_latency_E6_four_threads)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE quill_callsite_latency_E6_four_threads.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE quill)


set(TARGET_NAME quill_write_time_E5)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE quill_write_time_E5.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE quill)

set(TARGET_NAME quill_write_time_E6)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE quill_write_time_E6.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE quill)


# Run every executable
foreach(executable ${PROJECT_EXECUTABLES})
    add_test(NAME ${executable} COMMAND ${executable})
endforeach(executable)


