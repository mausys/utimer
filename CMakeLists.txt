cmake_minimum_required(VERSION 3.8)

project(check_utimer C)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 11)

find_package(Check REQUIRED)
find_package(Threads REQUIRED)

set(SRC
  utimer.c
  utimer_dmem.c
  sched_single.c
  check_utimer.c
)

add_executable(check_utimer ${SRC})
target_link_libraries(check_utimer Check::checkShared ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(check_utimer PRIVATE ${CHECK_INCLUDE_DIRS})
