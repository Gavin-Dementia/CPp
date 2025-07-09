# ConcurrentEngine_Light

A lightweight, modular and extensible C++ concurrency task scheduling engine with multi-strategy support.

This project aims to provide a customizable and efficient concurrency framework with multiple schedulers (FIFO, Priority, DAG), task encapsulation using `std::future`, and rejection policies for overload handling.

---

## 🌟 Features

- 🧵 **ThreadPool with dynamic worker management**
- 🗂️ **Multiple schedulers**:
  - FIFO (First-In-First-Out)
  - Priority Queue
  - DAG-based dependency scheduler
- 🔮 **Generic task encapsulation** using `std::packaged_task` / `std::function`
- 🚫 **Task rejection strategies**: BLOCK, DISCARD, THROW
- 📜 **Thread-safe logger**
- ✅ Modern C++ (C++20/23) design

---

## 🛠️ Build & Run

### 📦 Requirements

- C++20 or above
- CMake 3.14+

```bash
# Clone project
git clone https://github.com/Gavin-Dementia/ConcurrentEngine_Light.git
cd ConcurrentEngine_Light

# Build
mkdir build && cd build
cmake ..
make
./main
```

# Project Structure 
ConcurrentEngine_Light/

├── include/             # Public headers

├── src/                 # Core implementations

├── example/main.cpp     # Usage demo

├── docs/ARCHITECTURE.md # System architecture

├── CMakeLists.txt

├── .gitignore

└── README.md

# Example Task Example(Dependency Scheduling)
```bash
ThreadPool pool(4);

// Submit simple tasks
auto f1 = pool.submit([]() { return 100; });
auto f2 = pool.submit([]() { return 200; });
std::cout << "Sum: " << f1.get() + f2.get() << "\n";

// Submit with priority
pool.setScheduler(std::make_unique<PriorityScheduler>());
pool.submit([]() { std::cout << "High priority\n"; }, /*priority=*/10);

#Dag
DAGScheduler dag;
dag.addTask("A", [] { std::cout << "Task A\n"; });
dag.addTask("B", [] { std::cout << "Task B\n"; });
dag.addTask("C", [] { std::cout << "Task C\n"; });

// C depends on A and B
dag.addDependency("A", "C");
dag.addDependency("B", "C");

// Run tasks
dag.run();
```

# Execution Order (sample output)
```bash
Task A
Task B
Task C
```

# Modules Overview
Module	Description

ThreadPool : Manages worker threads and task submission
Scheduler : 	Abstract interface for task dispatching
FIFOScheduler :	Simple queue-based scheduler
PriorityScheduler	Priority : queue scheduling
DAGScheduler : 	Supports dependency-based task execution
Task	Encapsulation : of callable objects and results
Logger	: Thread-safe logging with timestamps

# License

This project is licensed under the MIT License. See LICENSE for details.
