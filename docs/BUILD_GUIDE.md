# Telemetry Platform - Build Guide

## Automated Build (Recommended)

### Quick Start with build.ps1

```powershell
# Auto-detect best Visual Studio version (2022 or 2026)
.\build.ps1

# Force specific Visual Studio version
.\build.ps1 -VSVersion 2022
.\build.ps1 -VSVersion 2026

# Build without tests
.\build.ps1 -BuildTests:$false
```

The script will:
1. Detect available VS version (2026 → 2022 → fail)
2. Configure CMake with GoogleTest + Protobuf
3. Build in Release mode
4. Run all unit tests (Redis mock + Protobuf)

## Manual Build with Visual Studio 2022/2026

### Prerequisites
- **Visual Studio 2022** (version 17.x) or **Visual Studio 2026** (version 18.x)
- CMake 3.14+
- Git
- Docker (optional, for Redis integration tests)

### Visual Studio Versions Supported

| VS Version | Generator String | CMake Version | Status |
|------------|------------------|---------------|--------|
| VS 2022    | `"Visual Studio 17 2022"` | 17.x | ✅ Stable |
| VS 2026    | `"Visual Studio 18 2026"` | 18.x | ✅ Bleeding Edge |
| VS 2019    | `"Visual Studio 16 2019"` | 16.x | ⚠️ Legacy |

### Build Steps

#### Option 1: Visual Studio 2022 (Stable)

```powershell
# 1. Configure with Visual Studio 2022 generator
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON

# 2. Build in Release mode
cmake --build build --config Release

# 3. Run unit tests (no Redis required - uses Google Mock)
.\build\common\Release\test_redis_client_unit.exe
.\build\common\Release\test_proto_adapter.exe

# Or run via CTest
cd build
ctest -C Release --output-on-failure
```

#### Option 2: Visual Studio 2026 (Bleeding Edge)

```powershell
# 1. Configure with Visual Studio 2026 generator
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTS=ON

# 2. Build and test (same as VS 2022)
cmake --build build --config Release
.\build\common\Release\test_redis_client_unit.exe
.\build\common\Release\test_proto_adapter.exe
```

### Build Options

```powershell
# Build only common library (no services)
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DBUILD_INGESTION=OFF -DBUILD_PROCESSING=OFF

# Build with tests
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DBUILD_TESTS=ON

# Build specific configuration
cmake --build build --config Debug
cmake --build build --config Release
```

### Test Targets

```powershell
# Build only unit tests (Google Test/Mock)
cmake --build build --target test_redis_client_unit --config Release

# Build integration test (requires Redis server)
cmake --build build --target test_redis_connection --config Release
```

### Troubleshooting

**Issue: "Generator not found"**
```powershell
# List available generators
cmake --help

# Use Visual Studio 2022 if 2019 not installed
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

**Issue: "Cannot open include file 'gtest/gtest.h'"**
- Solution: CMake will auto-download GoogleTest via FetchContent
- Ensure internet connection during first configuration

**Issue: Tests fail with "linking error"**
```powershell
# Clean and rebuild
Remove-Item -Recurse -Force build
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DBUILD_TESTS=ON
cmake --build build --config Release
```

## Interview Talking Points

### Why Visual Studio Generator?
- **Native Windows support**: MSVC is the primary compiler for Windows
- **IDE integration**: Open .sln file in Visual Studio for debugging
- **Multi-config**: Debug/Release built from same CMake configuration

### Why GoogleTest?
- **Industry standard**: Used by Google, Microsoft, AWS, Meta
- **Comprehensive**: Unit tests (gtest) + Mocking (gmock) in one framework
- **Mature**: 15+ years of development, battle-tested
- **CI/CD friendly**: XML output for Jenkins/Azure DevOps/GitHub Actions

### Build System Trade-offs
| Tool | Pros | Cons |
|------|------|------|
| CMake + MSVC | Native Windows, Visual Studio integration | Windows-only |
| CMake + Ninja | Fast builds, cross-platform | Requires separate Ninja install |
| MSBuild | Windows native, .NET integration | CMake better for C++ |

### Testing Strategy
- **Unit tests (gtest/gmock)**: 80% of tests, fast, isolated, no dependencies
- **Integration tests**: 20% of tests, require real Redis, verify end-to-end
- **No Redis required for development**: Mock Redis allows TDD without setup

---

**Next Steps:**
1. Build succeeds → Run unit tests
2. Unit tests pass → Review test coverage
3. Ready for Day 3 → Implement TaskQueue.cpp
