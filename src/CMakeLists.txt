cmake_minimum_required(VERSION 3.16)

project(quill_workshop2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GOOGLETEST_VERSION 1.14.0)

add_subdirectory(googletest)

option(LOG4CPP_SUPPRESS_LOG_LOCATION "Do not print source location in logs" OFF)
set(log4cpp_public_headers
    ./include/log4cpp/HexLogging.h
    ./include/log4cpp/ILogHandler.h
    ./include/log4cpp/Location.h
    ./include/log4cpp/Logger.h
    ./include/log4cpp/LogLevels.h
    ./include/log4cpp/LogMacros.h
    ./include/log4cpp/MockLogHandler.h
    ./include/log4cpp/Strings.h
)
add_library(log4cpp INTERFACE)
target_link_libraries(log4cpp INTERFACE ser4cpp)
target_include_directories(log4cpp INTERFACE ./include)

if(${LOG4CPP_SUPPRESS_LOG_LOCATION})
    target_compile_definitions(log4cpp INTERFACE LOG4CPP_SUPPRESS_LOG_LOCATION)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/quill/include
)
add_subdirectory(quill)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spdlog-1x spdlog)

# add_executable(workshop_exe2 main.cpp)
# target_link_libraries(workshop_exe2 PRIVATE quill::quill)

add_executable(gtest_runnable_quill gtest_runnable.cpp)
#target_compile_definitions(gtest_runnable_quill PUBLIC -DQUILL_X86ARCH)
target_link_libraries(gtest_runnable_quill PRIVATE quill::quill gtest_main)

#add_executable(gtest_runnable_spdlog gtest_runnable_spdlog.cpp)
#target_include_directories(gtest_runnable_spdlog PRIVATE ${CMAKE_SOURCE_DIR}/spdlog-1x/include)

#target_link_libraries(gtest_runnable_spdlog PRIVATE spdlog gtest_main)

#add_executable(gtest_runnable_4cpp gtest_runnable_4cpp.cpp)
#target_link_libraries(gtest_runnable_4cpp PRIVATE log4cpp gtest_main)

