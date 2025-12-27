# QA Verification Steps - TelemetryPlatform Monorepo

**Last Updated**: December 27, 2025  
**Build Status**: ✅ CMake configuration successful (146s)  
**Compiler**: MSVC 19.44.35222.0 (Visual Studio 2022)  
**Target**: Windows 10 SDK 10.0.26100.0  

---

## Prerequisites

### System Requirements
- Windows 10/11 (x64)
- Visual Studio 2022 with C++ desktop development
- CMake 3.14 or higher
- Git

### Installed Dependencies (via FetchContent)
- ✅ nlohmann/json v3.11.3
- ✅ hiredis master (v1.3.0+)
- ✅ redis-plus-plus master (v1.3.15)
- ✅ Google Protobuf v25.1
- ✅ cpp-httplib (latest)

---

## Step 1: Clean Build Verification

### 1.1 Clean Build Directory
```powershell
# From any location
Remove-Item -Recurse -Force c:\code\telemetry-platform\build\* -ErrorAction SilentlyContinue
```

### 1.2 CMake Configuration
```powershell
cmake -S c:\code\telemetry-platform `
      -B c:\code\telemetry-platform\build `
      -G "Visual Studio 17 2022" -A x64 `
      -DBUILD_TESTS=ON `
      -DBUILD_GUI=OFF
```

**Expected Result**:
```
-- Configuring done (120-180s)
-- Generating done (2-5s)
-- Build files have been written to: C:/code/telemetry-platform/build
```

**Success Criteria**:
- ✅ No CMake Error messages
- ✅ All FetchContent dependencies downloaded
- ✅ "Configuring done" message appears
- ⚠️ Deprecation warnings are acceptable (not failures)

### 1.3 Compile All Targets
```powershell
cmake --build c:\code\telemetry-platform\build --config Release
```

**Expected Duration**: 5-15 minutes (first build)

**Success Criteria**:
- ✅ Exit code 0
- ✅ No compilation errors
- ✅ Binaries created in `build\<component>\Release\`

---

## Step 2: Unit Tests Verification

### 2.1 Common Library Tests

#### Test: Redis Client (Mock Tests - No Redis Required)
```powershell
.\build\common\Release\test_redis_client_unit.exe
```

**Expected Output**:
```
[==========] Running 30 tests from 1 test suites.
[----------] Global test environment set-up.
[----------] 30 tests from RedisClientTest
[ RUN      ] RedisClientTest.ConnectionTest
[       OK ] RedisClientTest.ConnectionTest (0 ms)
...
[----------] 30 tests from RedisClientTest (X ms total)

[==========] 30 tests from 1 test suite ran. (X ms total)
[  PASSED  ] 30 tests.
```

**Success Criteria**:
- ✅ All 30 tests pass
- ✅ 0 failures
- ✅ Tests include: connection, set/get, batch operations, error handling

#### Test: Protobuf Adapter
```powershell
.\build\common\Release\test_proto_adapter.exe
```

**Expected Output**:
```
[==========] Running 11 tests from 1 test suites.
[----------] Global test environment set-up.
[----------] 11 tests from ProtoAdapterTest
[ RUN      ] ProtoAdapterTest.BasicSerialization
[       OK ] ProtoAdapterTest.BasicSerialization (0 ms)
[ RUN      ] ProtoAdapterTest.PerformanceBenchmark
[       OK ] ProtoAdapterTest.PerformanceBenchmark (X ms)
...
[----------] 11 tests from ProtoAdapterTest (X ms total)

[==========] 11 tests from 1 test suite ran. (X ms total)
[  PASSED  ] 11 tests.
```

**Success Criteria**:
- ✅ All 11 tests pass
- ✅ Serialization/deserialization works
- ✅ Performance test shows < 10μs per operation
- ✅ No memory leaks

#### Test: BoundedQueue
```powershell
.\build\common\Release\test_bounded_queue.exe
```

**Expected Output**:
```
[==========] Running X tests from 1 test suites.
[  PASSED  ] X tests.
```

**Success Criteria**:
- ✅ Thread-safety tests pass
- ✅ Capacity enforcement works
- ✅ Timeout behavior correct

#### Test: Config Parser
```powershell
.\build\common\Release\test_config.exe
```

**Expected Output**:
```
[==========] Running X tests from 1 test suites.
[  PASSED  ] X tests.
```

**Success Criteria**:
- ✅ INI file parsing works
- ✅ Default values applied correctly
- ✅ Error handling for malformed config

### 2.2 Test Summary Script
```powershell
# Run all tests and collect results
$tests = @(
    ".\build\common\Release\test_redis_client_unit.exe",
    ".\build\common\Release\test_proto_adapter.exe",
    ".\build\common\Release\test_bounded_queue.exe",
    ".\build\common\Release\test_config.exe"
)

foreach ($test in $tests) {
    if (Test-Path $test) {
        Write-Host "`n=== Running $test ===" -ForegroundColor Cyan
        & $test
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ PASSED" -ForegroundColor Green
        } else {
            Write-Host "❌ FAILED (Exit Code: $LASTEXITCODE)" -ForegroundColor Red
        }
    } else {
        Write-Host "⚠️ Test not found: $test" -ForegroundColor Yellow
    }
}
```

---

## Step 3: Integration Tests (Optional - Requires Docker)

### 3.1 Start Redis Container
```powershell
docker run -d --name redis-test -p 6379:6379 redis:7-alpine
```

### 3.2 Run Redis Integration Test
```powershell
.\build\common\Release\test_redis_connection.exe
```

**Expected Output**:
```
[==========] Running X tests from 1 test suites.
[----------] Global test environment set-up.
[----------] X tests from RedisIntegrationTest
[ RUN      ] RedisIntegrationTest.RealConnection
[       OK ] RedisIntegrationTest.RealConnection (X ms)
...
[  PASSED  ] X tests.
```

### 3.3 Cleanup
```powershell
docker stop redis-test
docker rm redis-test
```

---

## Step 4: Component-Specific Verification

### 4.1 Common Library
**Location**: `build/common/Release/telemetry_common.lib`

**Verify**:
```powershell
Test-Path build\common\Release\telemetry_common.lib
# Should return: True
```

**Check Symbols**:
```powershell
dumpbin /EXPORTS build\common\Release\telemetry_common.lib | Select-String "ProtoAdapter"
# Should show ProtoAdapter methods
```

### 4.2 Ingestion Service (TelemetryHub)
**Location**: `build/ingestion/Release/telemetryhub.exe`

**Verify**:
```powershell
Test-Path build\ingestion\Release\telemetryhub.exe
# Should return: True
```

**Quick Test**:
```powershell
.\build\ingestion\Release\telemetryhub.exe --version
# Should show version info
```

### 4.3 Processing Service (TelemetryTaskProcessor)
**Location**: `build/processing/Release/telemetry_processor.exe`

**Verify**:
```powershell
Test-Path build\processing\Release\telemetry_processor.exe
# Should return: True
```

---

## Step 5: Code Quality Checks

### 5.1 Naming Conventions (Day 2 Refactoring)
**Verify no namespace confusion**:

```powershell
# Check that TelemetrySampleCpp is used (not ambiguous TelemetrySample)
Get-ChildItem -Recurse -Filter "*.cpp" | Select-String "TelemetrySampleCpp" | Measure-Object
# Should show multiple matches

# Check ProtoAdapter type aliases
Get-Content common\include\telemetry_common\proto_adapter.h | Select-String "TelemetrySampleProto"
# Should show: using TelemetrySampleProto = telemetry::TelemetrySample;
```

### 5.2 Include Guard Check
```powershell
# All headers should have include guards
Get-ChildItem -Recurse -Path common\include -Filter "*.h" | ForEach-Object {
    $content = Get-Content $_.FullName -Raw
    if ($content -notmatch "#ifndef.*_H_") {
        Write-Host "Missing include guard: $($_.Name)" -ForegroundColor Red
    }
}
```

### 5.3 Memory Leak Check (Valgrind Alternative on Windows)
**Using Application Verifier (optional)**:
```powershell
# Install Application Verifier from Windows SDK
appverif -enable Heaps Handles -for test_proto_adapter.exe
.\build\common\Release\test_proto_adapter.exe
appverif -disable * -for test_proto_adapter.exe
```

---

## Step 6: Performance Verification

### 6.1 Protobuf Serialization Performance
**Expected**: < 10 microseconds per operation

Run the performance test:
```powershell
.\build\common\Release\test_proto_adapter.exe --gtest_filter=*Performance*
```

**Check Output**:
```
[ RUN      ] ProtoAdapterTest.PerformanceBenchmark
Serialization:   100000 iterations in X ms (Y μs/op)
Deserialization: 100000 iterations in X ms (Y μs/op)
[       OK ] ProtoAdapterTest.PerformanceBenchmark (X ms)
```

**Acceptance Criteria**:
- ✅ Serialization: < 10 μs/op
- ✅ Deserialization: < 10 μs/op
- ✅ Serialized size: < 100 bytes (typical telemetry sample)

### 6.2 BoundedQueue Throughput
**Expected**: > 1 million ops/sec (modern hardware)

```powershell
.\build\common\Release\test_bounded_queue.exe --gtest_filter=*Throughput*
```

---

## Step 7: Documentation Verification

### 7.1 Check Documentation Files
```powershell
$docs = @(
    "docs\BUILD_TROUBLESHOOTING.md",
    "docs\QA_VERIFICATION_STEPS.md",
    "docs\PROTOBUF_MIGRATION.md",
    "docs\BAUD_RATE_VISUAL_GUIDE.md",
    "docs\P99_OSCILLOSCOPE_MEASUREMENT.md",
    "docs\DAY_2_REFACTORING.md",
    "docs\DAY_2_COMPLETION_STATUS.md",
    "README.md",
    "CONTRIBUTING.md"
)

foreach ($doc in $docs) {
    if (Test-Path $doc) {
        Write-Host "✅ $doc exists" -ForegroundColor Green
    } else {
        Write-Host "❌ $doc missing" -ForegroundColor Red
    }
}
```

### 7.2 Verify Proto Files
```powershell
Test-Path common\proto\telemetry.proto
# Should return: True

# Check generated files after build
Test-Path build\telemetry.pb.h
Test-Path build\telemetry.pb.cc
```

---

## Step 8: Git Status Check

### 8.1 Check Uncommitted Changes
```powershell
cd c:\code\telemetry-platform
git status
```

**Expected Files Modified**:
- ✅ `common/CMakeLists.txt` (hiredis + redis++ integration)
- ✅ `common/include/telemetry_common/proto_adapter.h` (refactored types)
- ✅ `common/src/proto_adapter.cpp` (updated method signatures)
- ✅ `common/tests/test_proto_adapter.cpp` (updated type names)
- ✅ `docs/*.md` (new documentation files)

**Expected New Files**:
- ✅ `common/proto/telemetry.proto`
- ✅ `docs/BUILD_TROUBLESHOOTING.md`
- ✅ `docs/QA_VERIFICATION_STEPS.md`
- ✅ `docs/BAUD_RATE_VISUAL_GUIDE.md`
- ✅ `docs/P99_OSCILLOSCOPE_MEASUREMENT.md`
- ✅ `docs/DAY_2_REFACTORING.md`

### 8.2 Verify Correct Repository
```powershell
git remote -v
# Should show: origin = telemetry-platform (monorepo)
# NOT: telemetryhub (old repo)
```

---

## Step 9: Issue Regression Tests

### 9.1 Verify hiredis Header Path Issue is Fixed
```powershell
# Check that hiredis headers are copied to wrapper directory
Test-Path build\common\hiredis_wrapper\hiredis\hiredis.h
# Should return: True

# Verify redis++ can find headers
Get-Content build\_deps\redis++-src\CMakeLists.txt | Select-String "HIREDIS_FEATURE_TEST_HEADER"
# Should show it's looking for hiredis/hiredis.h (which now exists)
```

### 9.2 Verify No Symlink Issues
```powershell
# Symlinks were causing admin privilege errors
# Verify we're using file(COPY) instead
Get-Content common\CMakeLists.txt | Select-String "file\(COPY"
# Should show: file(COPY ${HIREDIS_HEADERS} DESTINATION ...)

# Should NOT contain:
Get-Content common\CMakeLists.txt | Select-String "CREATE_LINK"
# Should return nothing (or comment only)
```

---

## Common Issues & Solutions

### Issue 1: Tests Not Found
**Symptom**: `Test-Path .\build\common\Release\test_*.exe` returns False

**Solution**:
```powershell
# Rebuild with tests enabled
cmake -S c:\code\telemetry-platform -B c:\code\telemetry-platform\build -DBUILD_TESTS=ON
cmake --build c:\code\telemetry-platform\build --config Release
```

### Issue 2: DLL Not Found Errors
**Symptom**: `The code execution cannot proceed because XXX.dll was not found`

**Solution**:
```powershell
# Check build directory structure
Get-ChildItem -Recurse build\*.dll
# Ensure all DLLs are in PATH or same directory as executable
```

### Issue 3: CMake Cache Issues
**Symptom**: Changes to CMakeLists.txt not reflected

**Solution**:
```powershell
# Clear CMake cache
Remove-Item build\CMakeCache.txt
cmake -S c:\code\telemetry-platform -B c:\code\telemetry-platform\build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON -DBUILD_GUI=OFF
```

---

## QA Sign-Off Checklist

### Build System
- [ ] Clean CMake configuration succeeds
- [ ] Full build completes without errors
- [ ] All targets compile (common, ingestion, processing)
- [ ] Build time reasonable (< 20 min first build)

### Unit Tests
- [ ] test_redis_client_unit: 30/30 pass
- [ ] test_proto_adapter: 11/11 pass
- [ ] test_bounded_queue: All pass
- [ ] test_config: All pass

### Integration Tests (Optional)
- [ ] Redis integration test passes with Docker
- [ ] Can connect to real Redis instance
- [ ] Set/Get operations work end-to-end

### Code Quality
- [ ] No namespace confusion (TelemetrySampleCpp used consistently)
- [ ] Type aliases clear (TelemetrySampleProto)
- [ ] Include guards present
- [ ] No obvious memory leaks

### Performance
- [ ] Protobuf serialization < 10 μs/op
- [ ] BoundedQueue throughput > 1M ops/sec
- [ ] No performance regressions

### Documentation
- [ ] BUILD_TROUBLESHOOTING.md complete
- [ ] QA_VERIFICATION_STEPS.md (this file) complete
- [ ] All 7 Day 2 documentation files present
- [ ] README.md updated

### Git Hygiene
- [ ] Working in telemetry-platform repo (not telemetryhub)
- [ ] All changes staged for commit
- [ ] Commit message prepared
- [ ] No sensitive data in commits

---

## Final Verification Command

**Run all checks in one script**:

```powershell
# Save this as verify_all.ps1
$ErrorActionPreference = "Continue"
$passed = 0
$failed = 0

Write-Host "`n=== TelemetryPlatform QA Verification ===" -ForegroundColor Cyan

# 1. Build verification
if (Test-Path "build\common\Release\telemetry_common.lib") {
    Write-Host "✅ Common library built" -ForegroundColor Green
    $passed++
} else {
    Write-Host "❌ Common library missing" -ForegroundColor Red
    $failed++
}

# 2. Test executables
$tests = @("test_redis_client_unit.exe", "test_proto_adapter.exe")
foreach ($test in $tests) {
    if (Test-Path "build\common\Release\$test") {
        Write-Host "✅ $test exists" -ForegroundColor Green
        $passed++
    } else {
        Write-Host "❌ $test missing" -ForegroundColor Red
        $failed++
    }
}

# 3. Documentation
$docs = @("BUILD_TROUBLESHOOTING.md", "QA_VERIFICATION_STEPS.md")
foreach ($doc in $docs) {
    if (Test-Path "docs\$doc") {
        Write-Host "✅ $doc exists" -ForegroundColor Green
        $passed++
    } else {
        Write-Host "❌ $doc missing" -ForegroundColor Red
        $failed++
    }
}

# 4. hiredis wrapper fix
if (Test-Path "build\common\hiredis_wrapper\hiredis\hiredis.h") {
    Write-Host "✅ hiredis wrapper fix applied" -ForegroundColor Green
    $passed++
} else {
    Write-Host "❌ hiredis wrapper missing" -ForegroundColor Red
    $failed++
}

Write-Host "`n=== Summary ===" -ForegroundColor Cyan
Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor Red

if ($failed -eq 0) {
    Write-Host "`n🎉 All verification checks passed! Ready to commit." -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n⚠️ Some checks failed. Review before committing." -ForegroundColor Yellow
    exit 1
}
```

Run with:
```powershell
cd c:\code\telemetry-platform
.\verify_all.ps1
```

---

**QA Status**: 📋 Ready for execution  
**Estimated Time**: 30-45 minutes (full verification)  
**Last Reviewed**: December 27, 2025  
