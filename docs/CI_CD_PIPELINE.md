# CI/CD Pipeline Documentation

**Date:** January 2, 2026  
**Repository:** Telemetry-Platform (Microservices Architecture)

---

## Overview

This document describes the CI/CD pipeline for the **Telemetry-Platform** repository, which demonstrates production-ready microservices architecture with distributed coordination.

### Key Distinction from TelemetryHub

| Aspect | **TelemetryHub** | **Telemetry-Platform** |
|--------|------------------|------------------------|
| **Architecture** | Monolithic gateway + Qt GUI | Microservices (ingestion + processing) |
| **Language** | C++20 (modern features showcase) | C++17 (stable production) |
| **Focus** | Implementation mastery | Architecture patterns |
| **Coordination** | In-process queues | Redis-based distributed |
| **CI Focus** | GUI builds, FASTBuild, Qt6 | Microservices, integration, load testing |

---

## GitHub Actions Workflows

### 1. Microservices CI (`microservices-ci.yml`)

**Purpose:** Build and test microservices with sanitizers

**Jobs:**
- **Linux (ASAN+UBSAN):** Memory safety and undefined behavior detection
  - Runs on: `ubuntu-latest`
  - Preset: `linux-ninja-asan-ubsan`
  - Dependencies: Redis server for integration tests
  - Output: Test results, smoke test verification
  
- **Windows (MSVC):** Cross-platform compatibility
  - Runs on: `windows-latest`
  - Preset: `vs2022-release-ci`
  - Note: Some tests may fail without Redis (graceful degradation)
  
- **Coverage:** Code coverage analysis
  - Runs on: `ubuntu-latest`
  - Preset: `linux-ninja-coverage`
  - Tools: lcov, gcovr
  - Artifacts: HTML coverage report (7-day retention)
  
- **TSAN:** Thread safety and race condition detection
  - Runs on: `ubuntu-latest`
  - Preset: `linux-ninja-tsan`
  - Environment: `TSAN_OPTIONS="second_deadlock_stack=1 history_size=7"`

**Triggers:**
- Push to `main`/`master`
- Pull requests to `main`/`master`

**Success Criteria:**
- All tests pass on Linux (ASAN+UBSAN)
- Windows build succeeds
- Coverage > 70% (target: 90%+)
- No thread safety violations

---

### 2. Load Testing (`load-testing.yml`)

**Purpose:** Validate performance and scalability with k6

**Scenarios:**
1. **Smoke (10 VUs, 30s):** Health check, p95 < 50ms
2. **Baseline (100 VUs, 2m):** Establish baseline, p95 < 200ms
3. **Medium (1000 VUs, 2m):** Sustained medium load, p95 < 300ms
4. **High (5000 VUs, 5m):** Scalability target, p95 < 500ms

**Workflow:**
1. Install k6 from official repository
2. Start Redis server
3. Build ingestion gateway (Release preset)
4. Start gateway in background
5. Verify health endpoint
6. Run k6 load test
7. Upload results as artifacts

**Triggers:**
- Push to `main`/`master` (baseline scenario)
- Pull requests (smoke scenario)
- Manual dispatch (choose scenario)

**Artifacts:**
- k6 results (JSON + HTML)
- 7-day retention

---

### 3. Documentation (`docs.yml`)

**Purpose:** Generate and deploy Doxygen documentation to GitHub Pages

**Workflow:**
1. Install Doxygen and Graphviz
2. Update version from git tags
3. Generate HTML documentation
4. Deploy to GitHub Pages

**Triggers:**
- Push to `main`/`master`
- Manual dispatch

**Output:** https://amareshkumar.github.io/telemetry-platform/

---

## CMake Presets

### Linux Presets

| Preset | Purpose | Build Dir | Flags |
|--------|---------|-----------|-------|
| `linux-ninja-debug` | Development | build_debug | Debug, all warnings |
| `linux-ninja-release` | Production | build_release | Release, optimizations |
| `linux-ninja-asan-ubsan` | Memory safety | build_asan | AddressSanitizer + UBSan |
| `linux-ninja-tsan` | Thread safety | build_tsan | ThreadSanitizer |
| `linux-ninja-coverage` | Code coverage | build_coverage | gcov, no optimization |

### Windows Presets

| Preset | Purpose | Build Dir | Generator |
|--------|---------|-----------|-----------|
| `vs2022-debug` | Development | build_vs | VS 2022, Debug |
| `vs2022-release` | Production | build_vs | VS 2022, Release |
| `vs2022-release-ci` | CI/CD | build_ci | VS 2022, Release, clean env |

---

## Usage

### Local Development

```bash
# Configure and build with preset
cmake --preset linux-ninja-debug
cmake --build --preset linux-ninja-debug

# Run tests
ctest --preset linux-ninja-debug --output-on-failure

# Memory safety check (ASAN+UBSAN)
cmake --preset linux-ninja-asan-ubsan
cmake --build --preset linux-ninja-asan-ubsan
ctest --preset linux-ninja-asan-ubsan --output-on-failure

# Code coverage
cmake --preset linux-ninja-coverage
cmake --build --preset linux-ninja-coverage
ctest --preset linux-ninja-coverage --output-on-failure
lcov --capture --directory build_coverage --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### Windows Development

```powershell
# Configure and build with preset
cmake --preset vs2022-release
cmake --build --preset vs2022-release --config Release

# Run tests
ctest --preset vs2022-release --output-on-failure -C Release
```

### Load Testing (Requires Gateway Running)

```bash
# Build and start gateway
cmake --preset linux-ninja-release
cmake --build --preset linux-ninja-release
./build_release/ingestion/gateway/gateway_app &

# Verify health
curl http://localhost:8080/health

# Run k6 tests
k6 run --vus 10 --duration 30s tests/load/health_check.js          # Smoke
k6 run tests/load/high_concurrency_test.js                          # Baseline
k6 run --vus 1000 --duration 2m tests/load/high_concurrency_test.js # Medium
k6 run tests/load/telemetry_ingestion.js                            # High (5k VUs)
```

---

## Troubleshooting

### Issue: Coverage job fails with "gcov: error"

**Solution:** Ensure Debug build type with --coverage flags:
```cmake
CMAKE_CXX_FLAGS: "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
CMAKE_EXE_LINKER_FLAGS: "--coverage"
```

### Issue: TSAN reports data races in Redis client

**Expected:** External library false positives are acceptable. Focus on application code.

**Filter:** Use `TSAN_OPTIONS="suppressions=tsan.supp"` with suppression file.

### Issue: k6 test fails with "connection refused"

**Cause:** Gateway not running or not healthy

**Solution:**
1. Build gateway: `cmake --build --preset linux-ninja-release`
2. Start gateway: `./build_release/.../gateway_app &`
3. Verify: `curl http://localhost:8080/health`
4. Run k6: `k6 run tests/load/health_check.js`

### Issue: Windows tests fail without Redis

**Expected:** Some integration tests require Redis. Unit tests should pass.

**Solution:** Install Redis on Windows or skip integration tests:
```bash
ctest --preset vs2022-release-ci -E integration
```

---

## Interview Talking Points

### Why Different CI/CD from TelemetryHub?

**Answer:**
*"The CI/CD pipelines reflect the different architectural focuses:*

- **TelemetryHub CI/CD:** Validates implementation excellence - Qt6 GUI builds, FASTBuild optimization, C++20 features
- **Telemetry-Platform CI/CD:** Validates architecture patterns - Microservices coordination, Redis integration, distributed load testing

*This demonstrates I understand that CI/CD should match architectural goals, not just run tests."*

### Key Numbers to Memorize

- **Coverage Target:** 90%+ (current baseline TBD)
- **Load Test Scenarios:** 4 (10 VUs → 5000 VUs)
- **CI Jobs:** 4 (Linux, Windows, Coverage, TSAN)
- **Performance Target:** p95 < 200ms at 100 VUs
- **Scalability Target:** 5000 concurrent users

### Architectural Validation

*"The CI/CD pipeline validates production readiness:*
1. **Memory Safety:** ASAN+UBSAN catch leaks and undefined behavior
2. **Thread Safety:** TSAN validates distributed coordination
3. **Performance:** k6 validates 5000 VU scalability claim
4. **Integration:** Redis coordination tested in CI
5. **Cross-Platform:** Linux and Windows compatibility

*This proves the architecture scales, not just that it compiles."*

---

## Next Steps

### Priority 1 (Week 1)
- [ ] Run baseline k6 tests and document results
- [ ] Achieve >70% code coverage
- [ ] Fix any ASAN/TSAN violations
- [ ] Document actual performance numbers (replace TBD)

### Priority 2 (Week 2)
- [ ] Add Docker Compose integration to CI
- [ ] Add performance regression detection
- [ ] Create architecture visualization in CI
- [ ] Add security scanning (CodeQL, Snyk)

### Priority 3 (Future)
- [ ] Add Kubernetes smoke tests
- [ ] Add multi-region load testing
- [ ] Add chaos engineering tests (service failure injection)
- [ ] Add performance benchmarking over time

---

## References

- **Project Strategy:** `PROJECT_STRATEGY.md` - Why two repos
- **Project Boundaries:** `PROJECT_BOUNDARIES.md` - What belongs where
- **Performance Benchmarks:** `PERFORMANCE_BENCHMARKS.md` - k6 testing guide
- **GitHub Actions Docs:** https://docs.github.com/actions
- **k6 Documentation:** https://k6.io/docs/
- **CMake Presets:** https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html

---

**Last Updated:** January 2, 2026  
**Author:** Amaresh Kumar  
**Status:** ✅ CI/CD Infrastructure Complete, Awaiting First Test Runs
