/// @file Profiler.hpp
/// @brief Lightweight scoped function-level profiler for performance analysis.
///
/// Provides PROFILE_FUNCTION() and PROFILE_SCOPE() macros that measure
/// wall-clock time using std::chrono. Results are accumulated per-function
/// and printed as a sorted table on destruction. Controlled by the
/// ENABLE_PROFILING compile-time flag.
///
/// @author Ralph Muehleisen
/// @date 2026-01-26
/// @copyright Copyright Argonne National Laboratory
#pragma once
// Master switch for profiling. This will be defined via CMake for specific builds.
// If PROFILING_ENABLED is not defined or is 0, all profiling code compiles to nothing.
// Example: #define PROFILING_ENABLED 1

#if PROFILING_ENABLED

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace openstudio::isomodel::profiler {

// Helper to simplify the verbose function name from __PRETTY_FUNCTION__
inline std::string simplifyName(const char *raw_name) {
  std::string name(raw_name);
  auto parenPos = name.find('(');
  if (parenPos == std::string::npos) {
    return name; // Should not happen with a valid function signature
  }

  std::string prefix = name.substr(0, parenPos);

  auto scopePos = prefix.rfind("::");
  if (scopePos != std::string::npos) {
    // Find the scope before the last one to include the class name.
    auto classScopePos = prefix.rfind("::", scopePos - 1);
    if (classScopePos != std::string::npos) {
      return prefix.substr(classScopePos + 2);
    }
    return prefix.substr(scopePos + 2);
  }

  auto spacePos = prefix.rfind(' ');
  return (spacePos != std::string::npos) ? prefix.substr(spacePos + 1) : prefix;
}

struct ProfileResult {
  std::chrono::nanoseconds totalTime{0};
  size_t callCount = 0;
};

class Profiler {
public:
  // Singleton access
  static Profiler &getInstance() {
    static Profiler instance;
    return instance;
  }

  void addResult(const char *name, std::chrono::nanoseconds duration) {
    auto &result = m_results[name];
    result.totalTime += duration;
    result.callCount++;
  }

  void printResults(std::ostream &out) {
    // First, calculate the total time spent in all profiled scopes
    std::chrono::nanoseconds totalProfiledTime{0};
    for (const auto &[name, result] : m_results) {
      totalProfiledTime += result.totalTime;
    }
    double totalProfiledTime_ms =
        std::chrono::duration<double, std::milli>(totalProfiledTime).count();
    if (totalProfiledTime_ms < 1e-9)
      totalProfiledTime_ms = 1.0; // Avoid division by zero

    out << "\n--- Profiling Results ---\n";
    out << std::left << std::setw(50) << "Function" << std::right << std::setw(10) << "% Total"
        << std::setw(18) << "Total Time (ms)" << std::setw(12) << "Calls" << std::setw(20)
        << "Avg Time (us)\n";
    out << std::string(110, '-') << "\n";

    // Sort results by total time for a more useful report
    std::vector<std::pair<const char *, ProfileResult>> sortedResults(m_results.begin(),
                                                                      m_results.end());
    std::sort(sortedResults.begin(), sortedResults.end(),
              [](const auto &a, const auto &b) { return a.second.totalTime > b.second.totalTime; });

    for (const auto &[name, result] : sortedResults) {
      std::string simplifiedName = simplifyName(name);
      double total_ms = std::chrono::duration<double, std::milli>(result.totalTime).count();
      double percentage = (total_ms / totalProfiledTime_ms) * 100.0;
      double avg_us =
          std::chrono::duration<double, std::micro>(result.totalTime).count() / result.callCount;

      out << std::left << std::setw(50) << simplifiedName << std::right << std::fixed
          << std::setprecision(2) << std::setw(9) << percentage << "%" << std::fixed
          << std::setprecision(4) << std::setw(18) << total_ms << std::setw(12) << result.callCount
          << std::fixed << std::setprecision(4) << std::setw(20) << avg_us << "\n";
    }
    out << "-------------------------\n";
  }

private:
  // Private constructor/destructor for singleton pattern
  Profiler() = default;
  ~Profiler() = default;
  Profiler(const Profiler &) = delete;
  Profiler &operator=(const Profiler &) = delete;

  std::map<const char *, ProfileResult> m_results;
};

class ScopeTimer {
public:
  ScopeTimer(const char *name)
      : m_name(name), m_startTime(std::chrono::high_resolution_clock::now()) {}

  ~ScopeTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_startTime);
    Profiler::getInstance().addResult(m_name, duration);
  }

private:
  const char *m_name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};

} // namespace openstudio::isomodel::profiler

#define PROFILE_SCOPE(name) openstudio::isomodel::profiler::ScopeTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() \
  openstudio::isomodel::profiler::ScopeTimer timer##__LINE__(__PRETTY_FUNCTION__)

#else // PROFILING_ENABLED is 0 or not defined

#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()

#endif // PROFILING_ENABLED

