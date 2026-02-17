# ISOModel Refactoring — Session Context for Continuation

**Paste this entire document as your first message when starting a new Kilo Code task on the new machine.**

---

## Project Overview

This is a C++ building energy simulation library implementing ISO 13790 and related standards. The codebase is located at the git repo root, with source files in `IsoModel/src/`. We are systematically refactoring for consistency, readability, maintainability, and modern C++20 practice.

## Environment

- **C++ Standard:** C++20 (set in CMakeLists.txt)
- **Build System:** CMake 3.20 with FetchContent (GoogleTest v1.14.0, yaml-cpp 0.8.0)
- **Compiler:** GCC 13.3.0 on Ubuntu Linux (WSL)
- **Namespace:** `openstudio::isomodel` (with `Vector`/`Matrix` typedefs in `openstudio`)

## Build & Test Commands

```bash
# Build tests (from repo root or src directory):
cd build && cmake --build . --target isomodel_unit_tests -j$(nproc)

# Run tests:
cd build && ./isomodel_unit_tests

# If build directory doesn't exist yet:
mkdir -p build && cd build && cmake .. && cmake --build . --target isomodel_unit_tests -j$(nproc)
```

**Test baseline: 18/18 tests pass.** Always verify after each change.

## Master Plan Document

Read `REFACTORING_PLAN.md` in the source directory — it contains the full refactoring plan with 7 phases, completion status, and all technical decisions.

## Completed Work

### Phase 0: Preparation ✅
- Created `.clang-format` (LLVM-based, 2-space indent, Attach braces, 100-col limit, C++20)
- Ran `clang-format` on all source files
- Git commits: `c6587fc`, `68b87f1`

### Phase 1: Non-Breaking Cleanup ✅ (with deferrals)
- **1a.** `#pragma once` adopted for all headers (commit `c2077a4`)
- **1b.** Removed all `#ifdef ISOMODEL_STANDALONE` dead branches (commit `74199db`)
- **1c.** Replaced `#define TIMESLICES 8760` with `constexpr HOURS_IN_YEAR` from Constants.hpp; converted C-style arrays in TimeFrame.hpp to `std::array<int, HOURS_IN_YEAR>` (commit `5912c1c`)
- **1d.** Replaced `DEBUG_ISO_MODEL_SIMULATION` macro with `constexpr bool` in Constants.hpp (commit `5912c1c`)
- **1e.** Added `const` to all getter methods (commit `a2c2dc8`)
- **1f.** Added `[[nodiscard]]` to all getters and value-returning functions (commit `068c04f`)
- **1g.** Remove dead code/stale comments — **DEFERRED** (user decision: "I do not want to do phase 1g now")
- **1h.** Converted C-style arrays to `std::array` in SolarRadiation.hpp (commit `edb78d5`)
- **1i.** Standardize copyright headers — **DEFERRED**
- **1j.** Remove `old/` directory — **DEFERRED**

### REFACTORING_PLAN.md Updated ✅
- Marked Phase 0 and Phase 1 completion status (commit `9023615`)
- Revised Phase 2 scope (narrowed to `m_` prefix only)
- Revised Phase 4 (renamed from "Naming Convention Migration" to "Type Safety & Flag Cleanup")
- Updated §1 naming convention to preserve ISO equation notation

## Key Technical Decisions

1. **ISO equation variable names are PRESERVED** — Names like `dT_supp_cl()`, `eta_DC_COP()`, `H_ve()`, `E_pumps()` use underscores as subscript separators (not snake_case). They must NOT be renamed to camelCase because they enable cross-referencing with ISO standard documents.

2. **Dual naming convention:** Descriptive `camelCase` for English-named properties coexists with ISO equation notation. Both are legitimate.

3. **Member prefix:** Standardize on `m_` for all private/protected members. Currently `EndUses.hpp` uses `_` prefix, `Simulation.hpp` uses no prefix.

4. **`DEBUG_ISO_MODEL_SIMULATION`** is a `constexpr bool = false` in Constants.hpp. The compiler eliminates all debug blocks at compile time while keeping them syntax-checked.

5. **API backward compatibility** required (source-level, not binary). Extensions are OK. `[[deprecated]]` wrappers for transition.

## Next Steps: Phase 2

Phase 2 focuses on `m_` prefix standardization (internal, no API change):

- **2a.** `EndUses.hpp`: `_endUses` → `m_endUses`, `_valid` → `m_valid`
- **2b.** `UserModel.hpp`: `_edata` → `m_edata`, `_weather` → `m_weather`, `_weatherFilePath` → `m_weatherFilePath`
- **2c.** `Simulation.hpp`: Add `m_` prefix to protected members (`pop` → `m_pop`, `location` → `m_location`, etc.) and update all references in `UserModel.hpp`/`UserModel.cpp`
- **2d.** `HourlyModel` encapsulation: Move public members back to private, add const accessors
- **2e.** Add naming convention documentation

## Workflow Rules

**CRITICAL:** Wait for user to confirm/commit after each sub-task before proceeding to the next. The user commits manually and provides commit messages. Do NOT proceed to the next sub-task without explicit approval.

After each sub-task:
1. Make the code changes
2. Build and run tests to verify 18/18 pass
3. Provide a suggested commit message
4. **STOP and wait for user confirmation**

## Files of Interest

| File | Notes |
|------|-------|
| `REFACTORING_PLAN.md` | Master plan — read this first |
| `.clang-format` | Project formatting rules |
| `Constants.hpp` | `constexpr` constants, `DEBUG_ISO_MODEL_SIMULATION` |
| `Simulation.hpp` | Protected members with no `m_` prefix (Phase 2c target) |
| `EndUses.hpp` | `_` prefix members (Phase 2a target) |
| `UserModel.hpp` | `_` prefix members + references to Simulation members (Phase 2b/2c target) |
| `HourlyModel.hpp` | Public members that should be private (Phase 2d target) |
| `Cooling.hpp`, `Heating.hpp`, `Ventilation.hpp` | ISO equation notation accessors (preserved) |
| `Structure.hpp` | Mix of camelCase and ISO notation (preserved) |
| `SolarRadiation.hpp` | `std::array` members, `[[nodiscard]]` getters |
| `TimeFrame.hpp` | `std::array<int, HOURS_IN_YEAR>` members |

---

**To continue:** Start a new Kilo Code task in Code mode and paste this document. Then say: "Continue the ISOModel refactoring from Phase 2. Read REFACTORING_PLAN.md for the full plan."
