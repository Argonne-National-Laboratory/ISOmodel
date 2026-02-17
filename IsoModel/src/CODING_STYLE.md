# ISOModel C++ Coding Style Guide

This document defines the naming conventions and coding standards for the ISOModel C++ codebase.
For the full rationale and refactoring history, see `REFACTORING_PLAN.md` §1.

---

## 1. Naming Conventions

### Dual Naming Convention

The codebase uses two legitimate naming conventions that **coexist by design**:

| Convention | Use Case | Examples |
|------------|----------|----------|
| **Descriptive `camelCase`** | Properties with clear English names | `floorArea()`, `temperatureSetPointOccupied()`, `buildingHeight()` |
| **ISO equation notation** | Variables from ISO 13790 and related standards | `dT_supp_cl()`, `eta_DC_COP()`, `H_ve()`, `E_pumps()` |

ISO equation notation uses underscores as **subscript separators** (not snake_case).
These names are preserved because they enable direct cross-referencing with the ISO standard
documents and are expected by domain experts.

### Summary Table

| Entity | Convention | Example |
|--------|-----------|---------|
| Classes / Structs | `PascalCase` | `HourlyModel`, `SolarRadiation`, `HourlyCache` |
| Descriptive methods | `camelCase` | `floorArea()`, `buildingHeight()` |
| ISO equation methods | ISO notation | `dT_supp_cl()`, `eta_DC_COP()`, `H_ve()` |
| Private/protected members | `m_` + `camelCase` | `m_floorArea`, `m_cop` |
| ISO equation members | `m_` + ISO notation | `m_dT_supp_cl`, `m_eta_DC_COP` |
| Constants | `UPPER_SNAKE_CASE` | `HOURS_IN_YEAR`, `PI` |
| `constexpr` constants | `UPPER_SNAKE_CASE` | `HOURS_IN_YEAR` |
| Enum types | `PascalCase` | `FuelType` |
| Enum values | `PascalCase` | `FuelType::Electric` |
| Local variables | `camelCase` | `floorArea`, `totalGains` |
| Function parameters | `camelCase` | `aggregateByMonth`, `theta_air` |
| Namespaces | `lowercase` or `camelCase` | `openstudio::isomodel` |
| File names | `PascalCase.hpp` / `.cpp` | `HourlyModel.hpp`, `SolarRadiation.cpp` |

### Member Variable Prefix

All private and protected member variables use the `m_` prefix:

```cpp
// Descriptive members
double m_floorArea;
std::string m_weatherFilePath;

// ISO equation members
double m_dT_supp_cl;
double m_eta_DC_COP;
```

**Do not use** `_` prefix (reserved by the C++ standard in some contexts) or bare names
without a prefix for member variables.

---

## 2. Formatting

The project uses `clang-format` with the configuration in `.clang-format`:

- **Style base:** LLVM
- **Indent:** 2 spaces (no tabs)
- **Braces:** Attach (K&R style)
- **Column limit:** 100
- **Standard:** C++20

Run formatting before committing:

```bash
clang-format -i *.hpp *.cpp
```

---

## 3. Header Guards

Use `#pragma once` for all header files (no `#ifndef` guards):

```cpp
#pragma once

#include <vector>
// ...
```

---

## 4. Const-Correctness

- All getter methods must be `const`-qualified
- All getters and value-returning functions should be marked `[[nodiscard]]`
- Prefer `const&` return for non-trivial types

```cpp
[[nodiscard]] double floorArea() const noexcept { return m_floorArea; }
[[nodiscard]] const std::string& weatherFilePath() const noexcept { return m_weatherFilePath; }
```

---

## 5. ISO Equation Documentation

All ISO-notation accessors should have a Doxygen comment with the Unicode symbol and ISO reference:

```cpp
/// Supply temperature delta for cooling (ΔT_supp,cl) [K]. ISO 13790 §C.3.
[[nodiscard]] double dT_supp_cl() const noexcept { return m_dT_supp_cl; }

/// Ventilation heat transfer coefficient (H_ve) [W/K]. ISO 13790 §9.3.
[[nodiscard]] double H_ve() const noexcept { return m_H_ve; }
```

---

## 6. Modern C++ Practices

- **Standard:** C++20
- **Prefer `constexpr`** over `#define` for constants
- **Prefer `std::array`** over C-style arrays
- **Prefer `std::span`** for non-owning array views in function parameters
- **Use `std::shared_ptr`** for shared ownership (e.g., `EpwData`)
- **Use `std::move`** when transferring ownership of resources
- **Use `override`** on all virtual method overrides
- **Use `final`** on classes/structs not intended for inheritance

---

## 7. Access Specifiers

- Keep the public interface minimal — only expose what external code needs
- Order: `public` → `protected` → `private`
- Group related members together with comments

```cpp
class HourlyModel : public Simulation {
public:
  HourlyModel() noexcept;
  ~HourlyModel() override = default;

  [[nodiscard]] std::vector<EndUses> simulate(bool aggregateByMonth = false);

private:
  void initialize();
  // ... internal members ...
};
```

---

## 8. Debug Code

Use `constexpr bool` instead of preprocessor macros for debug flags:

```cpp
// In Constants.hpp
inline constexpr bool DEBUG_ISO_MODEL_SIMULATION = false;

// Usage — compiler eliminates dead code while keeping it syntax-checked
if constexpr (DEBUG_ISO_MODEL_SIMULATION) {
  std::cerr << "Debug: theta_air = " << theta_air << "\n";
}
```
