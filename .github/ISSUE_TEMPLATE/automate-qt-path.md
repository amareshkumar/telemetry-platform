---
name: Automate Qt PATH configuration for GUI deployment
about: Eliminate manual PATH setup for Qt dependencies
title: "[Enhancement] Auto-configure Qt PATH in CMake/deployment scripts"
labels: enhancement, build-system, documentation
assignees: ''
---

## 🎯 Problem
Currently, running `gui_app.exe` requires manually setting Qt bin directory in PATH:
```powershell
$env:PATH = "C:\Qt\6.10.1\msvc2022_64\bin;$env:PATH"
.\gui_app.exe
```

This is error-prone and not user-friendly for:
- New developers setting up the project
- CI/CD automated builds
- Release packaging

## 💡 Proposed Solutions

### Option 1: CMake Install Script (Recommended)
Add Qt DLL deployment to CMake install target:
```cmake
# gui/CMakeLists.txt
install(TARGETS gui_app DESTINATION bin)

# Copy Qt DLLs to install directory
if(Qt6_FOUND)
    get_target_property(Qt6Core_DLL Qt6::Core LOCATION)
    install(FILES ${Qt6Core_DLL} DESTINATION bin)
    # ... repeat for Gui, Widgets
    
    # Copy platform plugin
    install(FILES $<TARGET_FILE_DIR:Qt6::Core>/../plugins/platforms/qwindows.dll
            DESTINATION bin/platforms)
endif()
```

**Benefits**: Standard CMake workflow, works with `cmake --install`

### Option 2: Post-Build Custom Command
Auto-copy Qt DLLs after each build:
```cmake
add_custom_command(TARGET gui_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:gui_app>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:gui_app>
    # ...
)
```

**Benefits**: Automatic, no manual deployment step

### Option 3: windeployqt (Qt Official Tool)
Use Qt's deployment tool in CMake:
```cmake
find_program(WINDEPLOYQT windeployqt PATHS ${Qt6_DIR}/../../../bin)
add_custom_command(TARGET gui_app POST_BUILD
    COMMAND ${WINDEPLOYQT} $<TARGET_FILE:gui_app> --no-translations
)
```

**Benefits**: Handles all dependencies automatically, includes plugins

### Option 4: RPATH/RUNPATH (Linux-style, not Windows)
Not applicable to Windows, but worth noting for cross-platform builds.

### Option 5: Wrapper Script with Auto-PATH
Create `gui_app.bat` wrapper:
```batch
@echo off
set PATH=C:\Qt\6.10.1\msvc2022_64\bin;%PATH%
start "" "%~dp0gui_app.exe" %*
```

**Benefits**: Simple, no CMake changes needed

## 🔧 Implementation Steps

1. **Phase 1** (Quick Fix - Option 5):
   - [x] Create `deploy_qt_gui.ps1` (DONE in Day 2)
   - [ ] Create `gui_app.bat` launcher wrapper
   - [ ] Update documentation with launcher usage

2. **Phase 2** (Proper Fix - Option 3):
   - [ ] Add `windeployqt` integration to CMakeLists.txt
   - [ ] Test on clean Windows VM
   - [ ] Update CI/CD pipeline

3. **Phase 3** (Refinement):
   - [ ] Add CMake option: `TELEMETRY_AUTO_DEPLOY_QT` (default ON)
   - [ ] Handle different Qt versions (6.10.1, 6.11, etc.)
   - [ ] Document in [development.md](../docs/development.md)

## 📚 References
- Qt Deployment Guide: https://doc.qt.io/qt-6/windows-deployment.html
- CMake BundleUtilities: https://cmake.org/cmake/help/latest/module/BundleUtilities.html
- windeployqt docs: https://doc.qt.io/qt-6/windows-deployment.html#the-windows-deployment-tool

## 🎯 Acceptance Criteria
- [ ] Fresh clone works without manual PATH setup
- [ ] `cmake --build build --target gui_app` produces runnable executable
- [ ] `cmake --install build` creates deployable package
- [ ] CI/CD builds succeed without hardcoded paths
- [ ] Documentation updated with new workflow

## 🏷️ Priority
**Medium** - Workaround exists (deploy_qt_gui.ps1), but proper fix improves DX

## 🗓️ Target Milestone
**v0.3.0** (Day 5-7) - After core features stabilize

---

**Current Workaround** (Day 2):
```powershell
# Run once after build
.\tools\deploy_qt_gui.ps1

# Then gui_app.exe works directly
cd build_clean\ingestion\gui\Release
.\gui_app.exe
```
