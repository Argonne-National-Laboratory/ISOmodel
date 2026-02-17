# ISOModel C++ Refactoring Plan

## Executive Summary

This document is a comprehensive refactoring plan for the ISOModel C++ codebase. The goal is to achieve **consistency, readability, maintainability, and modern C++20 practice** while preserving **source-level API backward compatibility** (recompilation allowed, binary compatibility not required). API extensions are permitted where they improve clarity.

The codebase has already undergone partial modernization (C++20 standard, `constexpr` constants, `std::numbers`, `[[nodiscard]]`, `std::span`, etc.) but retains a mishmash of older patterns alongside newer ones. This plan identifies every category of inconsistency and proposes a phased approach to resolve them.

---

## Table of Contents

1. [Naming Convention Inconsistencies](#1-naming-convention-inconsistencies)
2. [Include Guard Modernization](#2-include-guard-modernization)
3. [Copyright Header Inconsistency](#3-copyright-header-inconsistency)
4. [Const-Correctness Gaps](#4-const-correctness-gaps)
5. [Member Variable Naming Prefix Inconsistency](#5-member-variable-naming-prefix-inconsistency)
6. [Getter/Setter Pattern Inconsistencies](#6-gettersetter-pattern-inconsistencies)
7. [Return-by-Value vs Return-by-Reference](#7-return-by-value-vs-return-by-reference)
8. [Preprocessor Macro Elimination](#8-preprocessor-macro-elimination)
9. [Access Specifier Discipline](#9-access-specifier-discipline)
10. [Smart Pointer Consistency](#10-smart-pointer-consistency)
11. [TimeFrame Modernization](#11-timeframe-modernization)
12. [MathHelpers Namespace & Organization](#12-mathhelpers-namespace--organization)
13. [Error Handling Strategy](#13-error-handling-strategy)
14. [Redundant Dual-Mode Code (`#ifdef ISOMODEL_STANDALONE`)](#14-redundant-dual-mode-code-ifdef-isomodel_standalone)
15. [UserModel Facade Simplification](#15-usermodel-facade-simplification)
16. [Modern C++ Idiom Adoption](#16-modern-c-idiom-adoption)
17. [Documentation Consistency](#17-documentation-consistency)
18. [Test Code Modernization](#18-test-code-modernization)
19. [Build System Cleanup](#19-build-system-cleanup)
20. [Dead Code Removal](#20-dead-code-removal)
21. [Implementation Phases](#21-implementation-phases)

---

## 1. Naming Convention Inconsistencies

**Problem:** The codebase mixes multiple naming conventions for the same kind of entity:

| Pattern | Examples | Files |
|---------|----------|-------|
| `camelCase` accessors | `floorArea()`, `temperatureSetPointOccupied()` | `Building.hpp`, `Structure.hpp` |
| ISO equation notation accessors | `dT_supp_cl()`, `T_cl_ctrl_flag()`, `eta_DC_COP()`, `win_ff()`, `p_exp()`, `H_ve()`, `E_pumps()`, `DC_YesNo()` | `Cooling.hpp`, `Heating.hpp`, `Ventilation.hpp`, `Structure.hpp` |
| `m_` prefix members | `m_floorArea`, `m_cop` | Most files |
| `_` prefix members | `_endUses`, `_valid`, `_edata`, `_weather` | `EndUses.hpp`, `UserModel.hpp` |
| No prefix members | `pop`, `location`, `lights`, `building` | `Simulation.hpp` |

**Revised Naming Convention (decided during Phase 2 discussion):**

The codebase has two legitimate naming conventions that should **coexist**:

1. **Descriptive `camelCase`** — for properties with clear English names: `floorArea()`, `temperatureSetPointOccupied()`, `buildingHeight()`
2. **ISO equation notation** — for properties that directly correspond to ISO 13790 (and related standard) equation variables: `dT_supp_cl()` (ΔT_supp,cl), `eta_DC_COP()` (η_DC,COP), `H_ve()` (H_ve), `E_pumps()` (E_pumps)

The ISO notation uses underscores as **subscript separators** (not snake_case), and these names should be **preserved** because:
- They allow direct cross-referencing with the ISO standard documents
- Renaming them to camelCase would obscure the mathematical meaning
- Domain experts expect these names

**Naming Standard:**
- **Descriptive properties:** `camelCase()` methods, `m_camelCase` members
- **ISO equation variables:** Preserve existing notation (e.g., `dT_supp_cl()`, `eta_DC_COP()`, `H_ve()`)
- **Member variable prefix:** Standardize on `m_` for all private/protected members
- **Constants:** `UPPER_SNAKE_CASE` (already consistent)
- **Enum values:** `PascalCase` (already consistent with `FuelType::Electric`)
- **Local variables:** `camelCase`

**Documentation requirement:** All ISO-notation accessors should have a Doxygen comment with the Unicode symbol and ISO reference:
```cpp
/// Supply temperature delta for cooling (ΔT_supp,cl) [K]. ISO 13790 §C.3.
[[nodiscard]] double dT_supp_cl() const noexcept { return m_dT_supp_cl; }
```

**Affected files (member prefix only):** `EndUses.hpp`, `UserModel.hpp`, `Simulation.hpp`

---

## 2. Include Guard Modernization

**Problem:** Two different include guard styles coexist:

| Style | Example | Files |
|-------|---------|-------|
| Standard `#ifndef` | `#ifndef ISOMODEL_BUILDING_HPP` | Most files |
| Double-underscore | `#ifndef __ISOMODEL_API_HPP__` | `ISOModelAPI.hpp` |

Double-underscore identifiers are **reserved by the C++ standard** (UB).

**Proposed Fix:** Standardize all guards to `ISOMODEL_FILENAME_HPP` pattern. Alternatively, adopt `#pragma once` (supported by all target compilers: GCC, Clang, MSVC) for simplicity.

**Recommendation:** Use `#pragma once` as the project already targets C++20 and modern compilers.

**Affected files:** All `.hpp` files, especially `ISOModelAPI.hpp`

---

## 3. Copyright Header Inconsistency

**Problem:** Copyright headers are inconsistent:

| Variant | Files |
|---------|-------|
| Full LGPL block (2008-2015) | `EndUses.hpp`, `MonthlyModel.hpp`, `ISOResults.hpp` |
| Full LGPL block (2008-2013) | `Location.hpp`, `Population.hpp`, `Ventilation.hpp`, `Structure.hpp`, `WeatherData.hpp`, `TimeFrame.hpp`, `EpwData.hpp` |
| Abbreviated block | `SimulationSettings.hpp`, `Ventilation.hpp` |
| No copyright header | `Building.hpp`, `Cooling.hpp`, `Heating.hpp`, `Lighting.hpp`, `Constants.hpp`, `MathHelpers.hpp`, `Profiler.hpp`, `Schedules.hpp`, `HourlyModel.hpp` |
| Refactoring comment block | `SolarRadiation.hpp`, `HourlyModel.hpp` |

**Proposed Fix:** Adopt a single, short SPDX-style header for all files:

```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2008-2025, Alliance for Sustainable Energy.
```

---

## 4. Const-Correctness Gaps

**Problem:** Many getter methods are missing `const` qualification:

| File | Non-const getters |
|------|-------------------|
| `UserModel.hpp` | `coolingPumpControl()`, `heatingPumpControl()`, `hvacCoolingLossFactor()`, `hvacHeatingLossFactor()`, `hvacWasteFactor()`, `heatGainPerPerson()`, `exteriorHeatCapacity()`, `roofArea()`, `skylightArea()`, `skylightSCF()`, `skylightSHGC()`, `skylightUvalue()`, `wallAreaE()` through `wallAreaW()`, `windowAreaE()` through `windowAreaW()`, and many more |
| `WeatherData.hpp` | `mEgh()`, `mdbt()`, `mwind()`, `msolar()`, `mhdbt()`, `mhEgh()` — all non-const |
| `SolarRadiation.hpp` | `surfaceTilt()`, `localMeridian()`, `lon()`, `lat()`, `groundReflectance()`, `monthlyDryBulbTemp()`, etc. |
| `EpwData.hpp` | `location()`, `stationid()` return by value but are fine; `data()` returns by value |

**Proposed Fix:** Add `const` to all getter methods that don't modify state. This is a source-compatible change.

**Affected files:** `UserModel.hpp`, `WeatherData.hpp`, `SolarRadiation.hpp`

---

## 5. Member Variable Naming Prefix Inconsistency

**Problem:** Three different prefix conventions are used:

| Convention | Files |
|------------|-------|
| `m_` prefix | `Building.hpp`, `Cooling.hpp`, `Heating.hpp`, `Lighting.hpp`, `Population.hpp`, `Ventilation.hpp`, `Structure.hpp`, `SimulationSettings.hpp`, `Location.hpp`, `SolarRadiation.hpp`, `HourlyModel.hpp` |
| `_` prefix | `EndUses.hpp` (`_endUses`), `UserModel.hpp` (`_valid`, `_edata`, `_weather`, `_weatherFilePath`) |
| No prefix | `Simulation.hpp` (`pop`, `location`, `lights`, `building`, `structure`, `heating`, `cooling`, `ventilation`, `epwData`, `simSettings`) |

**Proposed Fix:** Standardize on `m_` prefix for all private/protected member variables. Leading underscore followed by lowercase is technically legal but conventionally avoided.

**Affected files:** `EndUses.hpp`, `UserModel.hpp`, `Simulation.hpp`

---

## 6. Getter/Setter Pattern Inconsistencies

**Problem:** Multiple accessor patterns coexist:

| Pattern | Example | Files |
|---------|---------|-------|
| `value()` / `setValue()` | `floorArea()` / `setFloorArea()` | Most data classes |
| Return-by-value getter + separate `Ref()` getter | `wallArea()` + `wallAreaRef()` | `Structure.hpp`, `WeatherData.hpp` |
| Direct member access (protected) | `pop.setDaysStart(val)` | `Simulation.hpp`, `UserModel.hpp` |

The dual getter pattern (`wallArea()` returning a copy and `wallAreaRef()` returning `const&`) is verbose and error-prone.

**Proposed Fix:**
- Make the primary getter return `const&` (this is source-compatible since `const&` binds to the same expressions as by-value)
- Remove the `*Ref()` variants or mark them `[[deprecated]]`
- For `Simulation.hpp`, make protected members private and add accessors, or at minimum add `m_` prefix

```cpp
// Before:
Vector wallArea() const { return m_wallArea; }           // copies!
const Vector& wallAreaRef() const { return m_wallArea; } // reference

// After:
const Vector& wallArea() const { return m_wallArea; }    // reference (primary)
```

**Note:** This is source-compatible because any code doing `Vector v = obj.wallArea()` will still work (copy from const ref). Code doing `auto v = obj.wallArea()` will now get a `const Vector&` instead of `Vector`, which could change semantics if `v` is later modified. Audit call sites.

**Affected files:** `Structure.hpp`, `WeatherData.hpp`, `EpwData.hpp`

---

## 7. Return-by-Value vs Return-by-Reference

**Problem:** Several getters return expensive objects by value unnecessarily:

| Method | Return Type | File |
|--------|-------------|------|
| `EpwData::data()` | `std::vector<std::vector<double>>` (copy!) | `EpwData.hpp:80` |
| `WeatherData::mEgh()` etc. | `Vector` (copy) | `WeatherData.hpp` |
| `SolarRadiation::monthlyDryBulbTemp()` etc. | `std::vector<double>` (copy) | `SolarRadiation.hpp` |
| `SolarRadiation::eglobe()` | `std::vector<std::vector<double>>` (reconstructed) | `SolarRadiation.hpp` |

**Proposed Fix:** Return `const&` for all getters that return member data. Keep legacy reconstruction methods (like `eglobe()`) but mark them `[[deprecated]]` in favor of `eglobeFlat()`.

---

## 8. Preprocessor Macro Elimination

**Problem:** Several preprocessor macros should be replaced with modern C++ constructs:

| Macro | Location | Replacement |
|-------|----------|-------------|
| `#define TIMESLICES 8760` | `TimeFrame.hpp:12` | `inline constexpr int TIMESLICES = 8760;` or use `HOURS_IN_YEAR` from `Constants.hpp` |
| `#define DEBUG_ISO_MODEL_SIMULATION false` | `MathHelpers.hpp:75` | `inline constexpr bool DEBUG_ISO_MODEL_SIMULATION = false;` |
| `#define ISOMODEL_API` | `ISOModelAPI.hpp` | Keep (platform-specific, appropriate use of macros) |
| `#define PROFILE_SCOPE` / `#define PROFILE_FUNCTION` | `Profiler.hpp` | Keep (conditional compilation, appropriate use) |

**Proposed Fix:**
- Replace `TIMESLICES` with `HOURS_IN_YEAR` (already defined in `Constants.hpp`)
- Replace `DEBUG_ISO_MODEL_SIMULATION` macro with `constexpr bool`
- Keep platform/profiling macros as-is

**Affected files:** `TimeFrame.hpp`, `MathHelpers.hpp`

---

## 9. Access Specifier Discipline

**Problem:** `HourlyModel.hpp` has a problematic access pattern:

```cpp
private:
  void initialize();

public: // Changed from private to public
  void setPreloadedScheduleData(...);
  std::shared_ptr<EpwData> m_lastEpwData;        // public member!
  std::vector<double> m_cachedSolarRadiation;     // public member!
  std::vector<double> m_phi_H_nd;                 // public member!
  // ... many more public data members
```

Dozens of data members and implementation details are `public` with a comment "Changed from private to public". This breaks encapsulation.

**Proposed Fix:**
- Move all data members back to `private`
- Provide `const` accessors for any members that tests or external code need to read
- Make `setPreloadedScheduleData()` a proper public method (it's a legitimate API)
- If test code needs access, use `friend class` for the test fixture or provide test-only accessors

**Affected files:** `HourlyModel.hpp`

---

## 10. Smart Pointer Consistency

**Problem:** Mixed ownership patterns:

| Pattern | Example | File |
|---------|---------|------|
| `std::shared_ptr` | `Location::m_weather`, `Simulation::epwData` | `Location.hpp`, `Simulation.hpp` |
| Raw pointer (non-owning) | `SolarRadiation::m_frame`, `SolarRadiation::m_epwData` | `SolarRadiation.hpp` |
| Value semantics | `Simulation::pop`, `Simulation::building`, etc. | `Simulation.hpp` |

**Proposed Fix:**
- Document the ownership model explicitly
- Raw pointers in `SolarRadiation` are fine (non-owning, short-lived) but should use `std::span` or reference where possible, or at minimum be documented as non-owning
- Consider whether `Location::m_weather` truly needs `shared_ptr` or if `unique_ptr` suffices
- Consider using `std::observer_ptr` (or just raw pointer with comment) for non-owning references

---

## 11. TimeFrame Modernization

**Problem:** `TimeFrame` uses C-style arrays and a preprocessor macro:

```cpp
#define TIMESLICES 8760

class TimeFrame {
public:
  int YTD[TIMESLICES];
  int Hour[TIMESLICES];
  int DayOfMonth[TIMESLICES];
  int DayOfWeek[TIMESLICES];
  int Month[TIMESLICES];
};
```

This is the most C-style code in the project. Public raw arrays, macro constant, no encapsulation.

**Proposed Fix:**

```cpp
class TimeFrame {
public:
  TimeFrame();
  ~TimeFrame() = default;

  [[nodiscard]] int ytd(int hourOfYear) const { return m_ytd[hourOfYear]; }
  [[nodiscard]] int hour(int hourOfYear) const { return m_hour[hourOfYear]; }
  [[nodiscard]] int dayOfMonth(int hourOfYear) const { return m_dayOfMonth[hourOfYear]; }
  [[nodiscard]] int dayOfWeek(int hourOfYear) const { return m_dayOfWeek[hourOfYear]; }
  [[nodiscard]] int month(int hourOfYear) const { return m_month[hourOfYear]; }

  [[nodiscard]] static int monthLength(int month);

  // Provide span access for iteration
  [[nodiscard]] std::span<const int, HOURS_IN_YEAR> ytdSpan() const { return m_ytd; }
  // ... etc

private:
  std::array<int, HOURS_IN_YEAR> m_ytd{};
  std::array<int, HOURS_IN_YEAR> m_hour{};
  std::array<int, HOURS_IN_YEAR> m_dayOfMonth{};
  std::array<int, HOURS_IN_YEAR> m_dayOfWeek{};
  std::array<int, HOURS_IN_YEAR> m_month{};
};
```

**Backward compatibility:** Add `[[deprecated]]` public array references or keep old names as accessors during transition. Since this requires recompilation, direct array access like `frame.Hour[i]` would need to change to `frame.hour(i)`.

**Affected files:** `TimeFrame.hpp`, `TimeFrame.cpp`, `SolarRadiation.cpp`, `Schedules.cpp`

---

## 12. MathHelpers Namespace & Organization

**Problem:** `MathHelpers.hpp` defines `Vector` and `Matrix` in `namespace openstudio` but helper functions in `namespace openstudio::isomodel`. This split namespace is confusing.

```cpp
namespace openstudio {
  using Vector = std::vector<double>;
  class Matrix { ... };
}
namespace openstudio::isomodel {
  // All the math helper functions
}
```

Additionally, the file is a "kitchen sink" combining:
- Type aliases (`Vector`)
- A full class definition (`Matrix`)
- Debug printing utilities
- Vector/matrix arithmetic
- Conversion utilities

**Proposed Fix:**
- Move `Vector` typedef and `Matrix` class into `namespace openstudio::isomodel` (or a dedicated `Types.hpp`)
- Split into logical files:
  - `Types.hpp` — `Vector`, `Matrix` typedefs/classes
  - `MathHelpers.hpp` — arithmetic functions
  - `DebugPrint.hpp` — debug printing (or remove if unused)
- Use `namespace openstudio::isomodel` consistently

**Note:** The `Vector` typedef in `namespace openstudio` is used throughout. Moving it requires updating all files, but since recompilation is allowed, this is feasible.

---

## 13. Error Handling Strategy

**Problem:** Mixed error handling approaches:

| Approach | Example | File |
|----------|---------|------|
| `std::cerr` + continue | `std::cerr << "Failed to open EPW file"` | `EpwData.cpp:276` |
| `throw std::invalid_argument` | `throw std::invalid_argument("dhwFuelType...")` | `UserModel.hpp:444` |
| Silent default | `return 0.0` on out-of-bounds | `EndUses.hpp:53` |
| `catch(...)` swallow | `catch (...) { m_latitude = 0.0; }` | `EpwData.cpp:94` |

**Proposed Fix:**
- Define a project error handling policy:
  - **Programmer errors** (invalid indices, null pointers): Use `assert()` in debug, undefined behavior in release (or `std::terminate`)
  - **Runtime errors** (file not found, parse errors): Use exceptions (`std::runtime_error` or custom `ISOModelError`)
  - **Recoverable issues** (missing optional data): Use `std::optional` return types
- Replace `catch(...)` with specific exception types
- Replace `std::cerr` error reporting with exceptions or `std::expected` (C++23) / `std::optional`

---

## 14. Redundant Dual-Mode Code (`#ifdef ISOMODEL_STANDALONE`)

**Problem:** Multiple files have `#ifdef ISOMODEL_STANDALONE` blocks that maintain two parallel implementations:

| File | Dual-mode code |
|------|----------------|
| `EndUses.hpp` | Entirely different API (index-based vs enum-based) |
| `ISOResults.cpp` | Different iteration logic |
| `HourlyModel.hpp` | Different include paths |
| `MonthlyModel.hpp` | Different include paths + logger |
| `WeatherData.hpp` | Different include paths |
| `ISOModelFixture.hpp` | Different test setup |

Since `ISOMODEL_STANDALONE` is always defined (set in `CMakeLists.txt:147`), the non-standalone code paths are dead code.

**Proposed Fix:**
- Remove all `#ifndef ISOMODEL_STANDALONE` branches
- Remove the `#define ISOMODEL_STANDALONE` from CMakeLists.txt
- Simplify `EndUses` to have a single, clean API
- Remove OpenStudio-specific includes and logger references

**Risk:** If there's any intent to re-integrate with OpenStudio, keep the `#ifdef` blocks. Otherwise, remove them.

**Affected files:** `EndUses.hpp`, `ISOResults.cpp`, `HourlyModel.hpp`, `MonthlyModel.hpp`, `WeatherData.hpp`, `ISOModelFixture.hpp`, `ISOModelFixture.cpp`, `Structure.hpp`

---

## 15. UserModel Facade Simplification

**Problem:** `UserModel.hpp` is ~1600 lines, with ~1200 lines of trivial getter/setter forwarding methods like:

```cpp
double wallAreaE() { return structure.wallArea()[2]; }
void setWallAreaE(double val) { structure.setWallArea(2, val); }
// ... repeated for S, SE, E, NE, N, NW, W, SW × multiple properties
```

This creates a massive, hard-to-maintain facade with ~200+ forwarding methods.

**Proposed Fix:**
- Expose the sub-objects directly via const/non-const accessors:
  ```cpp
  const Structure& structure() const { return m_structure; }
  Structure& structure() { return m_structure; }
  ```
- Keep the most commonly used convenience methods
- Mark the per-direction forwarding methods as `[[deprecated]]` with guidance to use `structure().wallArea()[2]` directly
- Consider a direction enum to replace magic indices:
  ```cpp
  enum class Direction : int { S=0, SE=1, E=2, NE=3, N=4, NW=5, W=6, SW=7, Roof=8 };
  ```

**Affected files:** `UserModel.hpp`, `UserModel.cpp`

---

## 16. Modern C++ Idiom Adoption

### 16a. `[[nodiscard]]` Consistency

**Problem:** `[[nodiscard]]` is used in some places (`UserModel.hpp`, `HourlyModel.hpp`, `MathHelpers.hpp`) but not others.

**Proposed Fix:** Add `[[nodiscard]]` to all pure getter methods and functions that return computed values. This is a non-breaking addition.

### 16b. `noexcept` Consistency

**Problem:** `noexcept` is used on some methods (`Matrix::size1()`, `HourlyModel` constructor) but not on simple getters that clearly can't throw.

**Proposed Fix:** Add `noexcept` to all simple getters (return member by value/reference) and trivial functions.

### 16c. `std::string_view` for String Parameters

**Problem:** Some functions take `std::string` by value where `std::string_view` would be more efficient:

```cpp
void setScheduleFilePath(std::string scheduleFilePath) { ... }  // copies
void load(std::string buildingFile);                              // copies
```

**Proposed Fix:** Use `std::string_view` for input parameters that are only read, `std::string` by value + `std::move` for parameters that are stored.

### 16d. Structured Bindings and Range-Based For

Already used in some places (`Profiler.hpp`, `UserModel.hpp`). Ensure consistent use throughout.

### 16e. `enum class` for Boolean-Like Parameters

**Problem:** Several properties use `double` for what are conceptually boolean or enum values:

```cpp
double DC_YesNo() const;        // 0.0 or 1.0 — should be bool
double DH_YesNo() const;        // 0.0 or 1.0 — should be bool
double T_cl_ctrl_flag() const;  // flag — should be bool or enum
int vent_rate_flag() const;     // 0 or 1 — should be bool
```

**Proposed Fix:** Change to `bool` or appropriate `enum class`. Provide `[[deprecated]]` double-returning wrappers for backward compatibility.

### 16f. Replace C-style Arrays with `std::array`

**Problem:** `SolarRadiation.hpp` uses C-style arrays:
```cpp
double m_surfSin[NUM_VERTICAL_SURFACES] = {};
double m_surfCos[NUM_VERTICAL_SURFACES] = {};
```

**Proposed Fix:** Use `std::array<double, NUM_VERTICAL_SURFACES>`.

---

## 17. Documentation Consistency

**Problem:** Documentation style varies:

| Style | Example | Files |
|-------|---------|-------|
| Doxygen `/** ... */` | `/** Floor area (m2). */` | `Structure.hpp`, `Population.hpp` |
| C++ `///` | `/// Gets a Building property.` | `UserModel.hpp` |
| C `/* ... */` block | `/* SolarRadiation.hpp ... */` | `SolarRadiation.hpp`, `HourlyModel.hpp` |
| No documentation | Most getters in `Cooling.hpp`, `Heating.hpp` | Various |

**Proposed Fix:**
- Standardize on `///` (Doxygen-compatible single-line) for brief docs
- Use `/** ... */` for multi-line documentation blocks
- Add units and ISO standard references to all physical quantity getters:
  ```cpp
  /// Floor area [m²]
  double floorArea() const noexcept { return m_floorArea; }
  ```

---

## 18. Test Code Modernization

**Problem:**
- `ISOModelFixture` uses `virtual void SetUp() override` — the `virtual` keyword is redundant with `override`
- Test fixture uses old GTest patterns
- Magic numbers in test assertions without named constants

**Proposed Fix:**
- Remove redundant `virtual` keyword when `override` is present
- Use `EXPECT_DOUBLE_EQ` or `EXPECT_NEAR` consistently
- Extract test constants into named values

---

## 19. Build System Cleanup

**Problem:**
- `Makefile` (57KB!) coexists with `CMakeLists.txt` — likely auto-generated or legacy
- `cmake_install.cmake` is a generated file checked into source
- `old/` directory contains obsolete source files

**Proposed Fix:**
- Remove `Makefile` if CMake is the canonical build system
- Add `cmake_install.cmake` to `.gitignore`
- Remove or archive the `old/` directory
- Remove `test_data/old/` directory

---

## 20. Dead Code Removal

**Problem:** Several pieces of dead code exist:

| Item | Location |
|------|----------|
| Commented-out includes | `Structure.hpp:10-15`, `WeatherData.hpp:11-12` |
| `// REMOVED:` comments | `Simulation.hpp:15,38,55` |
| Unused `scheduleFilePath` property | `Population.hpp:84-88` (TODO from 2015) |
| `mainpage.hpp` (149 bytes) | Likely empty Doxygen stub |
| Virtual schedule methods returning 0 | `HourlyModel.hpp:174-180` — never overridden |

**Proposed Fix:** Remove all dead code, commented-out code, and stale TODO comments.

---

## 21. Implementation Phases

### Phase 0: Preparation (Low Risk) ✅ COMPLETE
- [x] Set up CI with the existing test suite to catch regressions
- [x] Run all tests, establish baseline (18/18 pass)
- [x] Create a `.clang-format` file encoding the chosen style
- [x] Run `clang-format` on all files for whitespace/brace consistency

### Phase 1: Non-Breaking Cleanup (No API Changes) ✅ COMPLETE (1g, 1i deferred)
- [x] **1a.** Fix include guards / adopt `#pragma once` (§2)
- [x] **1b.** Remove `#ifdef ISOMODEL_STANDALONE` dead branches (§14)
- [x] **1c.** Replace `#define TIMESLICES` with `HOURS_IN_YEAR` (§8)
- [x] **1d.** Replace `DEBUG_ISO_MODEL_SIMULATION` macro with `constexpr` (§8)
- [x] **1e.** Add `const` to all getters (§4)
- [x] **1f.** Add `[[nodiscard]]` to getters (§16a)
- [ ] **1g.** Remove dead code and stale comments (§20) — *deferred by decision*
- [x] **1h.** Fix C-style arrays → `std::array` in `SolarRadiation.hpp` (§16f)
- [ ] **1i.** Standardize copyright headers (§3) — *deferred*
- [ ] **1j.** Remove `old/` directory and generated files (§19) — *deferred*
- [x] All tests pass (18/18) after each sub-task

### Phase 2: Member Naming Consistency (Internal, No API Change)

**Scope (revised):** Only standardize the `m_` member variable prefix. ISO equation variable names (e.g., `dT_supp_cl`, `eta_DC_COP`, `H_ve`) are **preserved as-is** — see §1 for rationale.

- [ ] **2a.** Rename `_` prefix members to `m_` prefix in `EndUses.hpp` (`_endUses` → `m_endUses`, `_valid` → `m_valid`) (§5)
- [ ] **2b.** Rename `_` prefix members to `m_` prefix in `UserModel.hpp` (`_edata` → `m_edata`, `_weather` → `m_weather`, `_weatherFilePath` → `m_weatherFilePath`) (§5)
- [ ] **2c.** Add `m_` prefix to `Simulation.hpp` protected members (`pop` → `m_pop`, `location` → `m_location`, etc.) and update all references in `UserModel.hpp`/`UserModel.cpp` (§5)
- [ ] **2d.** Restore `HourlyModel` encapsulation — move public members to private, add const accessors (§9)
- [ ] **2e.** Add naming convention documentation to this file or a `CODING_STYLE.md`
- [ ] Run tests ✓

### Phase 3: Getter/Setter Modernization (Source-Compatible API Changes)
- [ ] Change return-by-value getters to return `const&` (§6, §7)
- [ ] Deprecate `*Ref()` methods (§6)
- [ ] Add `Direction` enum to replace magic indices (§15)
- [ ] Add `std::string_view` parameters where appropriate (§16c)
- [ ] Run tests ✓

### Phase 4: Type Safety & Flag Cleanup (API Extension + Deprecation)

**Scope (revised):** The original Phase 4 was "Naming Convention Migration" to rename ISO notation to camelCase. That has been **cancelled** — ISO notation is preserved. This phase now focuses on type safety improvements.

- [ ] Change `double` flags to `bool` / `enum class` with deprecated wrappers (§16e)
  - `DC_YesNo()` / `DH_YesNo()` → `bool` (keep double wrappers as `[[deprecated]]`)
  - `T_cl_ctrl_flag()` / `T_ht_ctrl_flag()` → `bool` or `enum class`
  - `vent_rate_flag()` → `bool`
- [ ] Run tests ✓

### Phase 5: Structural Refactoring
- [ ] Modernize `TimeFrame` class (§11)
- [ ] Split `MathHelpers.hpp` into `Types.hpp` + `MathHelpers.hpp` (§12)
- [ ] Unify namespace for `Vector`/`Matrix` (§12)
- [ ] Simplify `UserModel` facade — expose sub-objects, deprecate forwarding methods (§15)
- [ ] Standardize error handling (§13)
- [ ] Run tests ✓

### Phase 6: Documentation & Polish
- [ ] Standardize documentation style across all files (§17)
- [ ] Add units and ISO references to all physical quantity accessors
- [ ] Add ISO equation variable cross-reference comments to all ISO-notation accessors
- [ ] Modernize test code (§18)
- [ ] Final `clang-format` pass
- [ ] Run tests ✓

### Phase 7: Deprecation Removal (Future Release)
- [ ] Remove all `[[deprecated]]` wrappers
- [ ] Remove old naming variants
- [ ] Final cleanup

---

## Summary of Key Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Naming convention | Dual: `camelCase` for descriptive names, ISO notation preserved for equation variables | ISO names enable cross-referencing with standards; renaming would obscure meaning |
| Member prefix | `m_` for all private/protected members | Consistency; `_` prefix and no-prefix variants eliminated |
| Include guards | `#pragma once` | Simpler, universally supported |
| Return semantics | `const&` for member data | Avoids unnecessary copies |
| Error handling | Exceptions for runtime errors | Consistent, modern C++ |
| `ISOMODEL_STANDALONE` | Remove non-standalone branches | Dead code, always defined |
| Backward compatibility | `[[deprecated]]` wrappers during transition | Source-compatible migration path |
| Documentation | `///` single-line, `/** */` multi-line | Doxygen-compatible |

---

## Risk Assessment

| Phase | Risk | Mitigation |
|-------|------|------------|
| Phase 0 | Very Low | Formatting only | ✅ Complete |
| Phase 1 | Very Low | No API changes, only additions | ✅ Complete |
| Phase 2 | Low | Internal only (`m_` prefix), tests catch issues |
| Phase 3 | Medium | `const&` return could change `auto` deduction; audit call sites |
| Phase 4 | Low | Type safety with deprecated wrappers |
| Phase 5 | Medium-High | `TimeFrame` and `MathHelpers` changes touch many files |
| Phase 6 | Very Low | Documentation only |
| Phase 7 | High | Breaking change, requires version bump |

---

*Document prepared: 2025-02-16*
*Last updated: 2026-02-16 — Phase 0 & Phase 1 complete; Phase 2 scope revised*
*Codebase: ISOModel C++ (C++20, CMake 3.20)*
*Target: Source-compatible refactoring with deprecation path*
