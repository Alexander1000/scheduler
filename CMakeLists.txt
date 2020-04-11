cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)

project(scheduler)

include_directories(
    include
)

set(SCHEDULER_SOURCES
    src/main.cpp
    src/scheduler/resource.cpp
)

add_executable(
    ${PROJECT_NAME} src/main.cpp
    ${SCHEDULER_SOURCES}
)
