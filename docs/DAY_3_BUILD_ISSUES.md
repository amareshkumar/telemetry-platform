# Build Issues - Day 3 Morning

**Date:** December 28, 2025  
**Status:** FAILED (Exit Code 1)  
**Build Config:** `-DBUILD_GUI=ON` (but was actually OFF in cache)

---

## Critical Issues Found

### ❌ **Issue 1: Protobuf Compiler Crash**
**Error:**
```
error MSB8066: Custom build for 'telemetry.pb.h.rule' exited with code -1073741515
```

**Root Cause:**  
Exit code `-1073741515` = `0xC0000135` = **DLL not found**  
- `protoc.exe` built successfully but can't find `abseil_dll.dll` at runtime
- Protobuf v25.1 with Abseil dependency issue on Windows

**Impact:** Protobuf code generation fails → all dependent targets fail

---

### ❌ **Issue 2: GMock Linking Error**
**Error:**
```
test_redis_client_unit.obj : error LNK2019: unresolved external symbol 
"testing::internal::g_gmock_mutex"
"testing::internal::g_gmock_implicit_sequence"
```

**Root Cause:**  
`test_redis_client_unit` was NOT linked to `telemetry_common`
- Test includes `mock_redis_client.h` which uses GMock
- GMock symbols come transitively through `telemetry_common` → Protobuf → Abseil

**Fix Applied:**  
Updated [common/CMakeLists.txt](c:\code\telemetry-platform\common\CMakeLists.txt#L150-L161):
```cmake
target_link_libraries(test_redis_client_unit PRIVATE 
    telemetry_common  # ← ADDED: Provides protobuf + abseil transitive deps
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)
```

---

### ⚠️ **Issue 3: BUILD_GUI=OFF in Cache**
**Error:** No GUI build outputs

**Root Cause:**  
CMake cache still has `BUILD_GUI:BOOL=OFF` from previous configure
- Command line `-DBUILD_GUI=ON` ignored because cache takes precedence

**Fix:** Reconfigure with `-DBUILD_GUI=ON` or clean build

---

### ⚠️ **Issue 4: Gateway DLL vs Static Lib Confusion**
**Error:**
```
LINK : fatal error LNK1181: cannot open input file 'gateway_core.lib'
```

**Root Cause:**  
`gateway_core` built as **DLL** but tests expect **static .lib**
- Some projects linking against non-existent import library

**Note:** This cascades from Issue #1 (protobuf failure prevents proper build)

---

## Solution Plan

### **Step 1: Fix Protobuf Runtime PATH** (DLL discovery)
**Option A - Add to System PATH** (recommended for development):
```powershell
$env:PATH += ";C:\code\telemetry-platform\build_vs26\_deps\protobuf-build\Release"
$env:PATH += ";C:\code\telemetry-platform\build_vs26\bin\Release"
```

**Option B - Copy DLLs to protoc location:**
```powershell
$ProtocDir = "C:\code\telemetry-platform\build_vs26\_deps\protobuf-build\Release"
$DllSource = "C:\code\telemetry-platform\build_vs26\bin\Release\abseil_dll.dll"
Copy-Item $DllSource $ProtocDir
```

**Option C - Use static Protobuf** (cleanest):
Update [common/CMakeLists.txt](c:\code\telemetry-platform\common\CMakeLists.txt#L91-L96):
```cmake
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "Build static library" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Use dynamic MSVC runtime")
```

---

### **Step 2: Clean Rebuild with GUI**
```powershell
# Remove stale cache
Remove-Item -Recurse -Force build_vs26

# Reconfigure with GUI
cmake -B build_vs26 -G "Visual Studio 18 2026" -DBUILD_GUI=ON -DBUILD_TESTS=ON

# Build
cmake --build build_vs26 --config Release
```

---

### **Step 3: Verify Qt Configuration**
**Qt Installation:** ✅ Found at `C:\Qt\6.10.1`

**CMake Requirements:**
```cmake
find_package(Qt6 6.10.1 REQUIRED COMPONENTS Widgets Network)
```

**Expected Output:**
```
-- Found Qt6: C:/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6
```

If Qt not found, set `CMAKE_PREFIX_PATH`:
```powershell
cmake -B build_vs26 `
    -G "Visual Studio 18 2026" `
    -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/msvc2022_64" `
    -DBUILD_GUI=ON `
    -DBUILD_TESTS=ON
```

---

## Recommended Action (Quick Fix)

**Use the new build script:**
```powershell
.\build_with_gui.ps1 -Clean
```

**This script will:**
1. ✅ Clean stale build cache
2. ✅ Configure with `-DBUILD_GUI=ON`
3. ✅ Build with proper dependency order
4. ✅ Run tests with output-on-failure
5. ✅ Generate detailed log: `build_gui.txt`

---

## Technical Deep Dive

### Protobuf DLL Hell on Windows

**Why `-1073741515`?**
```cpp
// Windows error codes
#define STATUS_DLL_NOT_FOUND 0xC0000135
```

**What's missing?**
```
protoc.exe
  └─ abseil_dll.dll  ← Built to build_vs26\bin\Release\
       └─ Not in PATH when protoc runs during CMake build
```

**CMake Build Order:**
1. ✅ Build `abseil_dll.dll` → `build_vs26\bin\Release\`
2. ✅ Build `protoc.exe` → `build_vs26\_deps\protobuf-build\Release\`
3. ❌ Run `protoc.exe` to generate `.pb.h` → **CRASH** (can't find abseil_dll.dll)

**Why does it fail?**
- CMake executes `protoc.exe` in a **custom command** during build
- Custom command inherits parent process environment
- Parent process PATH doesn't include `build_vs26\bin\Release`
- Result: protoc can't load abseil_dll.dll

**Solution Options:**
1. **Static linking** - No DLL runtime dependency (cleanest)
2. **PATH manipulation** - Add DLL directory to environment
3. **DLL copying** - Put abseil_dll.dll next to protoc.exe

---

## Files Modified

1. [common/CMakeLists.txt](c:\code\telemetry-platform\common\CMakeLists.txt)
   - Added `telemetry_common` to `test_redis_client_unit` link libraries

2. [build_with_gui.ps1](c:\code\telemetry-platform\build_with_gui.ps1) ← **NEW**
   - Automated clean + configure + build + test
   - Handles GUI-specific configuration
   - Provides detailed timing and diagnostics

---

## Next Steps for Amaresh

**Before Day 3 Agenda:**
```powershell
# Quick fix - rebuild from scratch
.\build_with_gui.ps1 -Clean
```

**If build succeeds:**
- ✅ Continue with Day 3: Doxygen + TaskQueue

**If build still fails:**
- Review `build_gui.txt` for specific errors
- Check Qt6 CMake detection
- Verify Visual Studio 2026 toolchain

---

## Exit Codes Reference

| Exit Code | Hex         | Meaning                     | Common Cause                          |
|-----------|-------------|-----------------------------|---------------------------------------|
| 1         | 0x00000001  | Generic build failure       | Compilation or link errors            |
| -1073741515 | 0xC0000135 | DLL not found              | Missing runtime dependency (abseil)   |
| 0xc0000135 | 0xC0000135  | Same as above              | test_serial_port_sim.exe missing DLLs |

---

**Status:** Ready for clean rebuild  
**Estimated Time:** 8-12 minutes (clean configure + build + test)
