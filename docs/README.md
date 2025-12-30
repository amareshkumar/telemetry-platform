# Documentation Index

**TelemetryHub/TelemetryPlatform** - High-Performance C++ IoT Telemetry System

---

## 📋 Quick Navigation

### Getting Started
- [Build Guide](BUILD_GUIDE.md) - Complete build instructions for Windows/Linux
- [Architecture Overview](../ReadMe.md#architecture) - System design and components
- [Quick Start](../ReadMe.md#quick-start) - Run in 5 minutes

### Development Guides
- [Testing Setup](TESTING_SETUP_GUIDE.md) - GoogleTest, pytest, k6 configuration
- [End-to-End Testing](END_TO_END_TESTING_GUIDE.md) - Integration test strategy
- [Build Troubleshooting](BUILD_TROUBLESHOOTING.md) - Common issues and fixes
- [Profiling Guide](PROFILING_GUIDE.md) - Performance profiling with Visual Studio/Perf

### Technical Deep Dives
- [Architecture Diagrams](ARCHITECTURE_DIAGRAMS.md) - Visual system design
- [Protobuf Migration](PROTOBUF_MIGRATION.md) - Serialization optimization
- [Testing Frameworks Comparison](TESTING_FRAMEWORKS_COMPARISON.md) - GoogleTest vs Catch2 vs pytest
- [Documentation Tools](DOCUMENTATION_TOOLS_COMPARISON.md) - Doxygen vs alternatives

### API Documentation
- [Doxygen Setup Guide](DOXYGEN_SETUP_GUIDE.md) - Generate API docs locally
- **Live API Docs:** *(coming soon - GitHub Pages deployment in progress)*

---

## 📚 Progress Documentation

### Milestone Summaries
- [Day 3 Complete](DAY3_FINAL_SUMMARY.md) - TaskQueue implementation + testing
- [Day 4 Status](DAY4_STATUS.md) - Gateway REST API + monitoring stack

### Day-by-Day Logs (Development Journal)
<details>
<summary>Click to expand full history</summary>

- [Day 2 Summary](DAY_2_SUMMARY.md) - Build system fixes
- [Day 2 Protobuf](DAY_2_PROTOBUF_VS2026_SUMMARY.md) - VS2026 compatibility
- [Day 3 Progress](DAY3_PROGRESS.md) - TaskQueue development
- [Day 3 Build Success](DAY_3_BUILD_SUCCESS.md) - All tests passing
- [Day 4 Visual Summary](DAY4_VISUAL_SUMMARY.md) - ASCII art milestone view

</details>

---

## 🎓 Learning Resources

These documents demonstrate technical communication skills and help other engineers learn from this project:

### Interview Preparation Materials
- [Interview Quick Reference](INTERVIEW_QUICK_REFERENCE.md) - STAR stories, performance metrics
- [Day 4 Interview Notes](DAY4_INTERVIEW_NOTES.md) - Technical talking points
- [Google Test Cheat Sheet](GOOGLE_TEST_CHEAT_SHEET.md) - Unit testing patterns

**Why Public?** 
- Demonstrates ability to explain complex systems clearly
- Shows preparation methodology (valuable for hiring managers)
- Helps other engineers prepare for C++ infrastructure roles
- Provides real-world examples of system design interview answers

*Note: These are not "cheat sheets" for interviews, but rather structured notes on how to explain the actual technical work done in this project.*

---

## 🔧 Troubleshooting Guides
- [Build Troubleshooting](BUILD_TROUBLESHOOTING.md) - CMake, Protobuf, Qt issues
- [Protobuf Build Issue](PROTOBUF_BUILD_ISSUE.md) - Compiler crash workarounds
- [Git Commit Guide](GIT_COMMIT_GUIDE.md) - Conventional commits

---

## 📊 Technical Specifications

### Performance Metrics (Validated)
| Metric | Value | Tool |
|--------|-------|------|
| Gateway Throughput | 10-20 req/s baseline | k6 |
| Success Rate | 95.47% @ 100 VUs | k6 |
| p95 Latency | 1.87ms | k6 |
| Protobuf Serialization | 408k ops/sec | GoogleTest |
| Redis Operations | 50k SET/s, 60k GET/s | redis-benchmark |
| TaskQueue | O(log n) enqueue | GoogleTest |

### Technology Stack
- **Language:** C++17/C++20
- **Build:** CMake 3.28+, Visual Studio 2026
- **Serialization:** Protobuf 3.x (+ JSON fallback)
- **Queue:** Redis 7.x
- **Testing:** GoogleTest, pytest, k6
- **Monitoring:** Grafana + InfluxDB
- **Documentation:** Doxygen 1.10

---

## 🗂️ File Categories

```
docs/
├── Getting Started/        # Build guides, quick start
├── Development/            # Testing, profiling, troubleshooting
├── Architecture/           # Design docs, diagrams
├── Progress Logs/          # Day-by-day development journal
├── Learning Resources/     # Interview prep, cheat sheets
└── API Documentation/      # Doxygen setup, generated docs
```

---

## 🤝 Contributing

This is primarily a portfolio project, but issues and discussions are welcome for:
- Build system improvements
- Cross-platform compatibility
- Performance optimization ideas
- Documentation clarifications

---

## 📜 License

MIT License - See [LICENSE](../LICENSE) for details

---

## 📬 Contact

**Amaresh Kumar**
- GitHub: [@amareshkumar](https://github.com/amareshkumar)
- Project: [telemetry-platform](https://github.com/amareshkumar/telemetry-platform)

---

**Last Updated:** December 30, 2025  
**Project Version:** 5.0.0  
**Status:** Active Development - Day 5
