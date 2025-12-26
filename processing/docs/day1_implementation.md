# TelemetryTaskProcessor - Day 1 Implementation Plan

**Date**: December 25, 2025  
**Goal**: Project structure, CMake setup, Redis connection, basic Task struct

---

## Day 1 Objectives

- [x] Create project repository structure
- [ ] CMake build system configuration
- [ ] Redis client library integration
- [ ] Basic Task data structure
- [ ] Git repository initialization
- [ ] Initial commit with scaffolding

---

## Task Checklist

### 1. Project Structure Setup (30 min)

**Directory structure:**
```
DistQueue/
├── src/
│   ├── core/
│   │   ├── Task.h
│   │   ├── Task.cpp
│   │   └── RedisClient.h
│   ├── producer/
│   │   └── Producer.h
│   ├── worker/
│   │   └── Worker.h
│   └── main.cpp
├── include/
│   └── distqueue/
│       ├── Task.h
│       └── Config.h
├── tests/
│   ├── test_task.cpp
│   └── CMakeLists.txt
├── examples/
│   └── telemetry_simple_producer.cpp
├── docs/
│   ├── architecture.md
│   └── api.md
├── CMakeLists.txt
├── README.md
├── LICENSE
└── .gitignore
```

**Create directories:**
- [ ] src/core/
- [ ] src/producer/
- [ ] src/worker/
- [ ] include/distqueue/
- [ ] tests/
- [ ] examples/
- [ ] docs/

### 2. CMake Configuration (45 min)

**Root CMakeLists.txt features:**
- C++17 standard
- Redis client library (redis-plus-plus)
- JSON library (nlohmann/json)
- GoogleTest for testing
- Compiler warnings enabled
- Build types: Debug, Release

**Dependencies:**
- redis-plus-plus: Redis C++ client
- hiredis: Redis C client (dependency of redis-plus-plus)
- nlohmann/json: JSON serialization
- GTest: Unit testing

**Build configuration:**
- [ ] Set C++17 standard
- [ ] Find Redis libraries
- [ ] Find JSON library
- [ ] Configure test framework
- [ ] Set compiler flags

### 3. Redis Client Integration (60 min)

**RedisClient wrapper:**
- Connection management
- Error handling
- Connection pool (future)
- Basic operations: RPUSH, BLPOP, SET, GET

**Test Redis connection:**
```cpp
#include <sw/redis++/redis.h>

auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");
redis.ping(); // Should return "PONG"
```

**Implementation:**
- [ ] RedisClient.h header
- [ ] Connection initialization
- [ ] Basic command wrappers
- [ ] Error handling
- [ ] Connection test

### 4. Task Data Structure (45 min)

**Task.h definition:**
```cpp
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace telemetry_processor {

enum class Priority {
    HIGH = 0,
    NORMAL = 1,
    LOW = 2
};

enum class TaskStatus {
    SUBMITTED,
    RUNNING,
    COMPLETED,
    FAILED
};

struct Task {
    std::string id;           // UUID
    std::string type;         // "compute", "io", "notify"
    std::string payload;      // JSON serialized data
    Priority priority;
    int retry_count;
    int max_retries;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    
    // Serialization
    nlohmann::json to_json() const;
    static Task from_json(const nlohmann::json& j);
};

} // namespace telemetry_processor
```

**Implementation:**
- [ ] Task.h header file
- [ ] Task.cpp implementation
- [ ] JSON serialization (to_json)
- [ ] JSON deserialization (from_json)
- [ ] UUID generation helper

### 5. Git Repository Setup (15 min)

**Initialize Git:**
- [ ] git init
- [ ] Create .gitignore
- [ ] Initial commit
- [ ] Optional: Create GitHub repository

**.gitignore contents:**
```
# Build artifacts
build/
cmake-build-*/
*.o
*.so
*.a

# IDE
.vscode/
.idea/
*.swp

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake

# Tests
Testing/

# OS
.DS_Store
Thumbs.db
```

### 6. Documentation Stubs (30 min)

**Create initial documentation:**
- [ ] docs/architecture.md - System design overview
- [ ] docs/api.md - API reference stub
- [ ] docs/development.md - Build instructions
- [ ] LICENSE - MIT License

---

## Implementation Details

### CMakeLists.txt (Root)

```cmake
cmake_minimum_required(VERSION 3.20)
project(TelemetryTaskProcessor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(REDIS++ REQUIRED redis++)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# nlohmann/json - header-only
include(FetchContent)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# GoogleTest
enable_testing()
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Source files
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)
```

### Task.cpp (Basic Implementation)

```cpp
#include "telemetry_processor/Task.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace telemetry_processor {

std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; ++i) {
        ss << dis(gen);
        if (i == 7 || i == 11 || i == 15 || i == 19) {
            ss << "-";
        }
    }
    return ss.str();
}

nlohmann::json Task::to_json() const {
    return nlohmann::json{
        {"id", id},
        {"type", type},
        {"payload", payload},
        {"priority", static_cast<int>(priority)},
        {"retry_count", retry_count},
        {"max_retries", max_retries},
        {"created_at", std::chrono::system_clock::to_time_t(created_at)},
        {"updated_at", std::chrono::system_clock::to_time_t(updated_at)}
    };
}

Task Task::from_json(const nlohmann::json& j) {
    Task task;
    task.id = j["id"];
    task.type = j["type"];
    task.payload = j["payload"];
    task.priority = static_cast<Priority>(j["priority"]);
    task.retry_count = j["retry_count"];
    task.max_retries = j["max_retries"];
    task.created_at = std::chrono::system_clock::from_time_t(j["created_at"]);
    task.updated_at = std::chrono::system_clock::from_time_t(j["updated_at"]);
    return task;
}

} // namespace telemetry_processor
```

---

## Success Criteria (End of Day 1)

- [x] README.md created
- [ ] Project directory structure complete
- [ ] CMakeLists.txt builds successfully
- [ ] Redis client library integrated
- [ ] Task struct with JSON serialization
- [ ] At least one unit test passing
- [ ] Git repository initialized
- [ ] Documentation stubs created
- [ ] Build command works: `cmake -B build && cmake --build build`

---

## Next Steps (Day 2)

- Producer API implementation
- Task submission to Redis
- Unit tests for producer
- Example program (telemetry_simple_producer.cpp)

---

## Time Allocation

- Project structure: 30 min
- CMake configuration: 45 min
- Redis integration: 60 min
- Task implementation: 45 min
- Git setup: 15 min
- Documentation: 30 min
- Testing & debugging: 45 min

**Total: ~4 hours**

---

## Notes

**Redis installation required:**
- Windows: Use Docker for Redis
- Linux: `sudo apt-get install redis-server libhiredis-dev`
- Mac: `brew install redis hiredis`

**Build dependencies:**
- CMake 3.20+
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
- Redis server running

**If dependencies are missing:**
- Use vcpkg for Windows: `vcpkg install redis-plus-plus nlohmann-json gtest`
- Use FetchContent for header-only libraries

---

Let's start implementing! Tell me when you're ready to begin each section.
