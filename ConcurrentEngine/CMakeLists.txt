cmake_minimum_required(VERSION 3.16)
project(ConcurrentEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(include)

# Source files
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
)
# Source files
# file(GLOB_RECURSE SRC_NO_MAIN
#     src/*.cpp
# )
# list(REMOVE_ITEM SRC_NO_MAIN src/main.cpp)  # 移除 main.cpp

# Main executable
add_executable(main src/main.cpp ${SRC_FILES})

# Other test executables
# add_executable(priority_test examples/priority_test.cpp ${SRC_FILES})
# add_executable(dag_test examples/dag_test.cpp ${SRC_FILES})
# add_executable(future_return examples/future_return.cpp ${SRC_FILES})
# add_executable(reject_block examples/reject_block.cpp ${SRC_FILES})
# add_executable(reject_discard examples/reject_discard.cpp ${SRC_FILES})
# add_executable(reject_throw examples/reject_throw.cpp ${SRC_FILES})
