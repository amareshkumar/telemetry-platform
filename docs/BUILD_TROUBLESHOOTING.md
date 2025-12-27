# Build Troubleshooting Guide - Lessons Learned

**Date**: December 27, 2025  
**Issue**: CMake configuration failures with hiredis + redis-plus-plus integration  
**Attempts**: 12 iterations to successful build  
**Time Investment**: ~2 hours of debugging  

---

## The Problem

When integrating redis-plus-plus with hiredis via CMake FetchContent, we encountered a critical header path mismatch that caused 12 build failures before finding the solution.

### Root Cause

**redis-plus-plus expects hiredis headers in a subdirectory structure** (`hiredis/hiredis.h`), but **hiredis installs headers in its root directory** (`hiredis.h` directly).

This is a common pitfall when using FetchContent with dependencies that have expectations about directory layouts.

---

## Failed Attempts (What NOT to Do)

### ❌ Attempt 1-4: ZLIB and CMake version issues
```cmake
# Problem: protobuf_WITH_ZLIB not set
# Problem: protobuf_generate_cpp() not available with FetchContent
# Problem: redis++ tag 1.3.12 had old CMake requirements
# Problem: hiredis not fetched at all
```
**Lesson**: Always use latest stable branches (master) for modern CMake support.

### ❌ Attempt 5-10: Wrong HIREDIS_HEADER path
```cmake
set(HIREDIS_HEADER "${hiredis_SOURCE_DIR}" CACHE PATH "Path to hiredis headers")
# redis++ tried to find: ${HIREDIS_HEADER}/hiredis/hiredis.h
# Actual location: ${hiredis_SOURCE_DIR}/hiredis.h
# Result: File not found error
```
**Error message**:
```
CMake Error at build/_deps/redis++-src/CMakeLists.txt:153 (file):
  file failed to open for reading (No such file or directory):
    C:/code/telemetry-platform/build/_deps/hiredis-src/hiredis/hiredis.h
```

### ❌ Attempt 11: Symlink approach
```cmake
file(CREATE_LINK "${hiredis_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis" SYMBOLIC)
# Error: A required privilege is not held by the client.
```
**Lesson**: Symlinks require admin privileges on Windows. Avoid platform-specific solutions.

---

## ✅ The Solution (Attempt 12)

### Step 1: Copy hiredis headers to expected directory structure
```cmake
# Copy hiredis headers to a subdirectory so redis++ can find it as hiredis/hiredis.h
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis")
file(GLOB HIREDIS_HEADERS "${hiredis_SOURCE_DIR}/*.h")
file(COPY ${HIREDIS_HEADERS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis")
```

### Step 2: Point redis++ to the wrapper directory
```cmake
set(HIREDIS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper" CACHE PATH "Path to hiredis headers")
```

### Step 3: Use master branches for modern CMake
```cmake
# Hiredis - master branch
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# redis-plus-plus - master branch
FetchContent_Declare(
    redis++
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
```

### Step 4: Modern protobuf_generate() instead of deprecated function
```cmake
# Explicitly include the protobuf-generate module
include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")
protobuf_generate(
    TARGET telemetry_proto
    LANGUAGE cpp
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
```

---

## Key Takeaways

### 1. **Understand Dependency Expectations**
- Always check what directory structure a library expects for its dependencies
- Read CMakeLists.txt of dependencies to see how they find headers
- In redis++, line 152: `set(HIREDIS_FEATURE_TEST_HEADER "${HIREDIS_FEATURE_TEST_INCLUDE}/hiredis/hiredis.h")`

### 2. **Use `file(COPY)` for Directory Structure Mismatches**
```cmake
# Generic pattern for fixing header path mismatches:
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/wrapper/expected_subdir")
file(GLOB HEADERS "${source_dir}/*.h")
file(COPY ${HEADERS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/wrapper/expected_subdir")
set(DEPENDENCY_HEADER "${CMAKE_CURRENT_BINARY_DIR}/wrapper" CACHE PATH "Headers")
```

### 3. **Prefer Master Branches for Active Projects**
- Tagged releases may have outdated CMake code
- Master branches usually have modern CMake 3.14+ support
- Use `GIT_SHALLOW TRUE` to save bandwidth

### 4. **Avoid Platform-Specific Solutions**
- Symlinks work on Linux but fail on Windows without admin
- `file(COPY)` works everywhere
- Keep solutions cross-platform

### 5. **FetchContent Protobuf: Use Modern Functions**
```cmake
# ❌ OLD (doesn't work with FetchContent):
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# ✅ NEW (works with FetchContent):
include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")
protobuf_generate(TARGET telemetry_proto LANGUAGE cpp)
```

---

## Debugging Tips

### 1. Check actual file locations
```powershell
Get-ChildItem build\_deps\hiredis-src\ -Filter "*.h"
```

### 2. Read the dependency's CMakeLists.txt
```cmake
# In redis++-src/CMakeLists.txt line 152:
set(HIREDIS_FEATURE_TEST_HEADER "${HIREDIS_FEATURE_TEST_INCLUDE}/hiredis/hiredis.h")
# This tells you it expects hiredis/ subdirectory!
```

### 3. Enable verbose CMake output
```powershell
cmake -S . -B build --trace-expand | Select-String "hiredis"
```

### 4. Clean build directory between attempts
```powershell
Remove-Item -Recurse -Force build\* -ErrorAction SilentlyContinue
# Or even better: delete from C:\ to avoid file locks
cd c:\
Remove-Item -Recurse -Force c:\code\project\build\*
```

---

## Complete Working Configuration

**File**: `common/CMakeLists.txt`

```cmake
# JSON library
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

# Hiredis (required by redis-plus-plus) - using master for modern CMake
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
set(DISABLE_TESTS ON CACHE BOOL "Disable hiredis tests")
set(ENABLE_SSL OFF CACHE BOOL "Disable SSL")
FetchContent_MakeAvailable(hiredis)

# Copy hiredis headers to subdirectory for redis++ compatibility
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis")
file(GLOB HIREDIS_HEADERS "${hiredis_SOURCE_DIR}/*.h")
file(COPY ${HIREDIS_HEADERS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis")

# redis-plus-plus (modern C++ Redis client)
FetchContent_Declare(
    redis++
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
set(REDIS_PLUS_PLUS_BUILD_TEST OFF CACHE BOOL "Disable redis++ tests")
set(REDIS_PLUS_PLUS_BUILD_STATIC ON CACHE BOOL "Build static library")
set(REDIS_PLUS_PLUS_BUILD_SHARED OFF CACHE BOOL "Disable shared library")
set(HIREDIS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper" CACHE PATH "Path to hiredis headers")
set(HIREDIS_LIB hiredis CACHE STRING "Hiredis library target")
FetchContent_MakeAvailable(redis++)

# Google Protobuf
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v25.1
    GIT_SHALLOW TRUE
)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests")
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "Build static library")
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Use dynamic MSVC runtime")
set(protobuf_WITH_ZLIB OFF CACHE BOOL "Disable ZLIB dependency")
FetchContent_MakeAvailable(protobuf)

# Generate .proto files using modern protobuf_generate
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
add_library(telemetry_proto STATIC ${PROTO_FILES})
target_link_libraries(telemetry_proto PUBLIC protobuf::libprotobuf)
target_include_directories(telemetry_proto PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")
protobuf_generate(
    TARGET telemetry_proto
    LANGUAGE cpp
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
```

---

## Time Investment Analysis

| Attempt | Issue | Time Lost |
|---------|-------|-----------|
| 1-2 | ZLIB + protobuf_generate_cpp | 15 min |
| 3-4 | CMake version compatibility | 20 min |
| 5-10 | Wrong HIREDIS_HEADER path | 60 min |
| 11 | Symlink permission error | 10 min |
| 12 | ✅ file(COPY) solution | 5 min |
| **Total** | | **~110 min** |

**If we had this guide**: 5 minutes (direct to solution)  
**ROI**: This documentation saves 105 minutes for future developers.

---

## Verification Commands

After successful configuration:

```powershell
# 1. Verify CMake succeeded
cmake -S c:\code\telemetry-platform -B c:\code\telemetry-platform\build `
    -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON -DBUILD_GUI=OFF

# 2. Build all targets
cmake --build c:\code\telemetry-platform\build --config Release

# 3. Run unit tests
.\build\common\Release\test_redis_client.exe
.\build\common\Release\test_proto_adapter.exe
```

---

## Future Improvements

1. **Upstream fix**: Submit PR to redis-plus-plus to auto-detect hiredis layout
2. **CMake find module**: Create FindHiredis.cmake that handles both layouts
3. **Documentation**: Add this to redis-plus-plus wiki
4. **CI/CD**: Add Windows build to redis-plus-plus GitHub Actions

---

## Related Issues

- redis-plus-plus GitHub: No issues found (yet)
- This is a common FetchContent + Windows problem
- Similar pattern seen with other header-only or C libraries

---

**Documented by**: AI Assistant + Human Developer (Paired Programming)  
**Status**: ✅ Resolved - Build successful on attempt 12  
**Build time**: 146 seconds (configure) + compilation time  
