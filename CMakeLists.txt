cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)

project(scheduler)

include_directories(
    include
)

set(SCHEDULER_SHARED_SOURCES
    src/scheduler/resource.cpp
    src/scheduler/item.cpp
    src/scheduler/bucket.cpp
    src/scheduler/scheduler.cpp
)

set(SCHEDULER_SOURCES
    src/main.cpp
    ${SCHEDULER_SHARED_SOURCES}
)

add_executable(
    ${PROJECT_NAME} src/main.cpp
    ${SCHEDULER_SOURCES}
)

#############
### TESTS ###
#############

project(scheduler-test)

set(CMAKE_BUILD_TYPE Debug)

set(SCHEDULER_SOURCES_TEST
    tests/main.cpp
    ${SCHEDULER_SHARED_SOURCES}
)

include_directories(include)

find_library(CPP_UNIT_TEST libcpp-unit-test.a)
message(STATUS "Found cpp-unit-test: ${CPP_UNIT_TEST}")

find_library(IO_BUFFER libio-buffer.a)
message(STATUS "Found io-buffer library: ${IO_BUFFER}")

find_library(YAML_PARSER libyaml-parser.a)
message(STATUS "Found yaml-parser library: ${YAML_PARSER}")

find_library(SHELL_GRID libshell-grid.a)
message(STATUS "Found shell-grid: ${SHELL_GRID}")

add_executable(
    ${PROJECT_NAME} tests/main.cpp
    ${SCHEDULER_SOURCES_TEST}
)

target_link_libraries(${PROJECT_NAME}
    ${CPP_UNIT_TEST}
    ${IO_BUFFER}
    ${YAML_PARSER}
    ${SHELL_GRID}
)
