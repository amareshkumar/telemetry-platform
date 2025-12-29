cmake_minimum_required(VERSION 3.14)

project(TelemetryPlatform 
    VERSION 1.0.0
    DESCRIPTION "Integrated Telemetry Platform: Ingestion + Processing"
    LANGUAGES CXX
)

# C++17 standard for consistency
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "=================================================")
message(STATUS "Telemetry Platform - Monorepo Build")
message(STATUS "=================================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "")

# Options
option(BUILD_INGESTION "Build TelemetryHub ingestion service" ON)
option(BUILD_PROCESSING "Build TelemetryTaskProcessor processing service" ON)
option(BUILD_COMMON "Build common shared library" ON)
option(BUILD_TESTS "Build all tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_CATCH2_TESTS "Build Catch2 BDD-style tests" ON)
option(BUILD_DOCS "Build Doxygen documentation" OFF)
option(BUILD_GUI "Build Qt GUI for ingestion service" ON)  # Added to root for consistency

# Build order: common first (dependency for both projects)
if(BUILD_COMMON)
    message(STATUS "Building: common/ (shared library)")
    add_subdirectory(common)
endif()

# Build ingestion service (TelemetryHub)
if(BUILD_INGESTION)
    message(STATUS "Building: ingestion/ (TelemetryHub)")
    add_subdirectory(ingestion)
endif()

# Build processing service (TelemetryTaskProcessor)
if(BUILD_PROCESSING)
    message(STATUS "Building: processing/ (TelemetryTaskProcessor)")
    add_subdirectory(processing)
endif()

# Integration tests (optional)
if(BUILD_TESTS)
    enable_testing()
    message(STATUS "Building: tests/")
    add_subdirectory(tests)
endif()

message(STATUS "")
message(STATUS "=================================================")
message(STATUS "Configuration Summary:")
message(STATUS "  Common Library:       ${BUILD_COMMON}")
message(STATUS "  Ingestion Service:    ${BUILD_INGESTION}")
message(STATUS "  Processing Service:   ${BUILD_PROCESSING}")
message(STATUS "  Tests:                ${BUILD_TESTS}")
message(STATUS "  Examples:             ${BUILD_EXAMPLES}")
message(STATUS "=================================================")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  cmake --build build --target telemetry_common      (common library)")
message(STATUS "  cmake --build build --target telemetry_gateway     (ingestion service)")
message(STATUS "  cmake --build build --target telemetry_processor   (processing service)")
message(STATUS "  cmake --build build                                (build all)")

# Documentation with Doxygen
if(BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        message(STATUS "")
        message(STATUS "Doxygen found: ${DOXYGEN_VERSION}")
        message(STATUS "  Documentation target available: cmake --build . --target docs")
        
        # Configure Doxygen
        set(DOXYGEN_PROJECT_NAME "TelemetryHub")
        set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})
        set(DOXYGEN_PROJECT_BRIEF "High-performance IoT telemetry platform")
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/doxygen)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_LATEX YES)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_EXTRACT_PRIVATE YES)
        set(DOXYGEN_EXTRACT_STATIC YES)
        set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
        set(DOXYGEN_SOURCE_BROWSER YES)
        set(DOXYGEN_REFERENCED_BY_RELATION YES)
        set(DOXYGEN_REFERENCES_RELATION YES)
        set(DOXYGEN_CALL_GRAPH YES)
        set(DOXYGEN_CALLER_GRAPH YES)
        set(DOXYGEN_HAVE_DOT YES)
        set(DOXYGEN_UML_LOOK NO)
        set(DOXYGEN_TEMPLATE_RELATIONS YES)
        set(DOXYGEN_COLLABORATION_GRAPH YES)
        set(DOXYGEN_CLASS_GRAPH YES)
        set(DOXYGEN_INCLUDE_GRAPH YES)
        set(DOXYGEN_INCLUDED_BY_GRAPH YES)
        set(DOXYGEN_GRAPHICAL_HIERARCHY YES)
        set(DOXYGEN_DIRECTORY_GRAPH YES)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_SOURCE_DIR}/README.md)
        
        # Add documentation target
        doxygen_add_docs(
            docs
            ${CMAKE_SOURCE_DIR}/common/include
            ${CMAKE_SOURCE_DIR}/common/src
            ${CMAKE_SOURCE_DIR}/ingestion/gateway/include
            ${CMAKE_SOURCE_DIR}/ingestion/gateway/src
            ${CMAKE_SOURCE_DIR}/processing/include
            ${CMAKE_SOURCE_DIR}/processing/src
            ${CMAKE_SOURCE_DIR}/README.md
            ${CMAKE_SOURCE_DIR}/docs/architecture.md
            COMMENT "Generating API documentation with Doxygen"
        )
    else()
        message(WARNING "")
        message(WARNING "Doxygen not found. Documentation will not be generated.")
        message(WARNING "  Install: choco install doxygen.install graphviz")
        message(WARNING "  Or download: https://www.doxygen.nl/download.html")
    endif()
endif()

message(STATUS "=================================================")
