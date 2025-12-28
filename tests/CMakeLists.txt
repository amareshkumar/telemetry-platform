# Tests Directory - Multi-Framework Testing Infrastructure
# Day 3 Extended: Catch2 + k6 + pytest scaffolding

cmake_minimum_required(VERSION 3.14)

message(STATUS "")
message(STATUS "=================================================")
message(STATUS "Testing Infrastructure Setup")
message(STATUS "=================================================")

# ========== Catch2 BDD-Style Tests ==========
if(BUILD_CATCH2_TESTS)
    message(STATUS "Configuring: Catch2 tests (BDD-style)")
    
    # Fetch Catch2 v3 from GitHub
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2  # Latest stable as of Dec 2025
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    
    # Add Catch2 CMake modules to path
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    
    # Create Catch2 test subdirectory
    add_subdirectory(catch2)
    
    message(STATUS "  Catch2 v3.5.2 configured")
    message(STATUS "  Run: ctest -C Release -R Catch2")
else()
    message(STATUS "Catch2 tests: DISABLED (enable with -DBUILD_CATCH2_TESTS=ON)")
endif()

# ========== Integration Tests (pytest) ==========
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration/pytest.ini")
    message(STATUS "")
    message(STATUS "Integration tests (pytest) detected:")
    message(STATUS "  Run: cd tests/integration && pytest -v")
else()
    message(STATUS "")
    message(STATUS "Integration tests (pytest): Scaffolding created")
    message(STATUS "  Setup: cd tests/integration && pip install -r requirements.txt")
    message(STATUS "  Run: pytest -v")
endif()

# ========== Load Tests (k6) ==========
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/load/telemetry_load.js")
    message(STATUS "")
    message(STATUS "Load tests (k6) detected:")
    message(STATUS "  Run: k6 run tests/load/telemetry_load.js")
else()
    message(STATUS "")
    message(STATUS "Load tests (k6): Scaffolding created")
    message(STATUS "  Install k6: https://k6.io/docs/getting-started/installation/")
    message(STATUS "  Run: k6 run tests/load/telemetry_load.js --vus 100 --duration 30s")
endif()

message(STATUS "=================================================")
message(STATUS "")
