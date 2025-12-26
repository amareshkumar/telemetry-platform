#pragma once
#include <iostream>

namespace telemetryhub {

inline const char* version()  { return "4.0.0"; }
inline const char* git_tag()  { return "v1.0.0"; }
inline const char* git_sha()  { return "monorepo"; }

static inline void print_version() {
  std::cout << "TelemetryHub " << version()
            << " (git: " << git_tag()
            << ", " << git_sha() << ")\n";
}

static inline void print_help(const char* prog) {
  std::cout << "Usage: " << (prog ? prog : "gateway_app") << " [options]\n"
            << "  -v, --version    Print version and git info\n"
            << "  -h, --help       Show this help\n";
}

} // namespace telemetryhub
