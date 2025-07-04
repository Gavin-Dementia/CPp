# ConcurrentEngine - Modern C++ Task Scheduling Framework

A lightweight and extensible thread pool framework built with **Modern C++23**, supporting custom **schedulers**, **task rejection policies**, and future-based **asynchronous task** execution.

---

## 🚀 Features

- 🧩 **Pluggable Scheduling** (FIFO, Priority, DAG-ready)
- 🚧 **Rejection Policies**: BLOCK, DISCARD, THROW
- 🧵 **Thread Pool Modes**: FIXED / CACHED (SINGLE planned)
- 📦 **Futures + packaged_task** for return values
- 🧠 **Thread Metadata**: Track thread IDs, state, lifecycle

---

## 🔧 Architecture Overview

| Component        | Description |
|------------------|-------------|
| `IScheduler`     | Interface for custom schedulers |
| `FIFOScheduler`  | Basic first-in-first-out queue |
| `PriorityScheduler` | High, Medium, Low task priority |
| `DAGScheduler` *(WIP)* | Supports DAG-based task dependency |
| `ThreadPool`     | Unified task engine with mode/rejection control |

---

## 📂 Directory Structure

ConcurrentEngine/
├── include/ # Public headers
├── src/ # Core implementations
├── examples/ # Demo & test programs
├── gui/ # Qt GUI monitor (optional)
├── legacy/ # Old versions (for reference)
└── README.md # You're here


---

## 🧪 Example Usage

```cpp
ThreadPool pool(std::make_unique<PriorityScheduler>());
pool.start(4);

for (int i = 0; i < 10; ++i) {
    pool.submit(i % 2 == 0 ? TaskPriority::HIGH : TaskPriority::LOW,
                [i] { std::cout << "Task " << i << " running\n"; });
}


Task Rejection Policies
Test different behaviors when task queue is full:

testRejectPolicy(RejectPolicy::BLOCK, "BLOCK");
testRejectPolicy(RejectPolicy::DISCARD, "DISCARD");
testRejectPolicy(RejectPolicy::THROW, "THROW");

Console Output (示例):
--- policy: DISCARD ---
[FIFOScheduler] Task discarded (queue full)

--- policy: THROW ---
[FIFOScheduler] Task rejected (queue full)
terminate called after throwing ...


Build with G++
g++ -std=c++23 -Iinclude \
    src/threadPool.cpp src/core/thread.cpp src/core/thread_meta.cpp \
    src/scheduler/priorityScheduler.cpp src/logger/threadlogger.cpp \
    examples/priority_test.cpp -o priority_test.exe -pthread

🗂️ To Do
 Qt GUI 

 DAG 調度整合進主體

 Profiler: performance analized

 CUDA/GPU task managing（processing）

