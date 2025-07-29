cmake_minimum_required(VERSION 3.16)
project(ConcurrentEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(include)

# Gather all library source files
file(GLOB_RECURSE LIB_SOURCES
    src/*.cpp
)

# === [ Core Library ] ===
add_library(ConcurrentEngineLib ${LIB_SOURCES})

# === [ Example Executables ] ===
function(add_example name)
    add_executable(${name} examples/${name}.cpp)
    target_link_libraries(${name} PRIVATE ConcurrentEngineLib)
endfunction()

# 單檔案範例
# add_example(main)
add_example(dag_test)
add_example(future_return)
add_example(reject_block)
add_example(reject_discard)
add_example(reject_throw)

# === [ Mini Redis Example ] ===
# file(GLOB_RECURSE MINI_REDIS_SOURCES
#     examples/mini_redis/*.cpp
# )

# add_executable(mini_redis ${MINI_REDIS_SOURCES})
# target_link_libraries(mini_redis PRIVATE ConcurrentEngineLib)
