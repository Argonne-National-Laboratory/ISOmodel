#!/usr/bin/env python3
"""
Generate Doxygen-compatible file header information from git history.

For each source file, extracts:
- Filename
- List of authors (from git log)
- Date of first commit
- Existing file header comment (if any) for context

Output: file_headers.txt with draft headers for review before insertion.
"""

import subprocess
import os
import sys
from pathlib import Path

# Files to process (relative to IsoModel/src/)
SOURCE_FILES = [
    # Headers
    "Building.hpp",
    "Constants.hpp",
    "Cooling.hpp",
    "EndUses.hpp",
    "EpwData.hpp",
    "Heating.hpp",
    "HourlyModel.hpp",
    "ISOModelAPI.hpp",
    "ISOResults.hpp",
    "Lighting.hpp",
    "Location.hpp",
    "mainpage.hpp",
    "MathHelpers.hpp",
    "MonthlyModel.hpp",
    "Population.hpp",
    "Profiler.hpp",
    "Schedules.hpp",
    "Simulation.hpp",
    "SimulationSettings.hpp",
    "SolarRadiation.hpp",
    "Structure.hpp",
    "TimeFrame.hpp",
    "UserModel.hpp",
    "Ventilation.hpp",
    "WeatherData.hpp",
    # Source files
    "EpwData.cpp",
    "HourlyModel.cpp",
    "ISOResults.cpp",
    "MonthlyModel.cpp",
    "Schedules.cpp",
    "SolarRadiation.cpp",
    "standalone_main.cpp",
    "Structure.cpp",
    "TimeFrame.cpp",
    "UserModel.cpp",
    # Test files
    "Test/HourlyModel_GTest.cpp",
    "Test/HourlySchedules_GTest.cpp",
    "Test/ISOModel_Benchmark.cpp",
    "Test/ISOModel_GTest.cpp",
    "Test/ISOModelFixture.cpp",
    "Test/ISOModelFixture.hpp",
    "Test/MonthlyModel_GTest.cpp",
    "Test/OptimizationCoverage_GTest.cpp",
    "Test/solar_debug.cpp",
    "Test/SolarRadiation_GTest.cpp",
    "Test/TimeFrame_GTest.cpp",
    "Test/UserModel_GTest.cpp",
]

# Git repo root
REPO_ROOT = "/home/rmuehleisen/git/ISOmodel"
SRC_DIR = os.path.join(REPO_ROOT, "IsoModel", "src")

# Normalize author names (git config variations → canonical name)
AUTHOR_MAP = {
    "muehleisen": "Ralph Muehleisen",
}


def normalize_author(name):
    """Map variant git author names to canonical form."""
    return AUTHOR_MAP.get(name, name)


def git_log_authors(filepath):
    """Get unique authors for a file, ordered by first contribution."""
    # Use git log --follow to track renames
    result = subprocess.run(
        ["git", "log", "--follow", "--format=%aN", "--", filepath],
        capture_output=True, text=True, cwd=REPO_ROOT
    )
    if result.returncode != 0:
        return []

    # Preserve order of first appearance, deduplicate
    seen = set()
    authors = []
    # Reverse so earliest commits come first
    for name in reversed(result.stdout.strip().split('\n')):
        name = normalize_author(name.strip())
        if name and name not in seen:
            seen.add(name)
            authors.append(name)
    return authors


def git_first_commit_date(filepath):
    """Get the date of the first commit that touched this file."""
    result = subprocess.run(
        ["git", "log", "--follow", "--format=%ai", "--diff-filter=A", "--", filepath],
        capture_output=True, text=True, cwd=REPO_ROOT
    )
    if result.returncode != 0 or not result.stdout.strip():
        # Fallback: get the oldest commit date
        result = subprocess.run(
            ["git", "log", "--follow", "--reverse", "--format=%ai", "--", filepath],
            capture_output=True, text=True, cwd=REPO_ROOT
        )
        if result.returncode != 0 or not result.stdout.strip():
            return "Unknown"
        # First line is the oldest
        return result.stdout.strip().split('\n')[0].split(' ')[0]

    dates = result.stdout.strip().split('\n')
    # Take the last one (oldest, since git log is newest-first)
    return dates[-1].split(' ')[0]


def get_existing_header(filepath):
    """Read the first few lines to see if there's already a header comment."""
    full_path = os.path.join(SRC_DIR, filepath)
    if not os.path.exists(full_path):
        return "(file not found)"

    with open(full_path, 'r') as f:
        lines = f.readlines()

    header_lines = []
    for line in lines[:15]:
        stripped = line.rstrip()
        if stripped.startswith('//') or stripped == '':
            header_lines.append(stripped)
        elif stripped.startswith('#pragma once') or stripped.startswith('#include') or stripped.startswith('#ifndef'):
            break
        else:
            break

    return '\n'.join(header_lines).strip() if header_lines else "(no header)"


def generate_summary(filepath):
    """Generate a brief summary based on filename and content analysis."""
    name = os.path.basename(filepath)
    base = os.path.splitext(name)[0]

    # Each entry is (brief, detailed_description)
    # The brief goes on the @brief line; the detailed description follows after a blank ///.
    summaries = {
        "Building": (
            "Building-level properties for internal gains and controls.",
            "Holds appliance power densities (electric and gas, occupied and unoccupied),\n"
            "lighting occupancy sensor and constant illumination control multipliers,\n"
            "building energy management (BEM) temperature adjustment, and external\n"
            "equipment energy use. Used by both MonthlyModel and HourlyModel."
        ),
        "Constants": (
            "Physical and mathematical constants used throughout the ISO model.",
            "Defines constexpr values for PI, unit conversions (hours, days, months),\n"
            "physical constants (Stefan-Boltzmann, air density, specific heat),\n"
            "safe epsilon values, and debug flags. Replaces former preprocessor macros\n"
            "with type-safe C++20 constexpr constants."
        ),
        "Cooling": (
            "Cooling system properties and HVAC distribution parameters.",
            "Stores COP, partial load value, temperature setpoints (occupied and\n"
            "unoccupied), HVAC loss factors, pump power, and district cooling\n"
            "parameters. Properties map to ISO 13790 and EN 15243 cooling calculations."
        ),
        "EndUses": (
            "Energy end-use category indices for simulation results.",
            "Defines the EndUses enum with indices for heating, cooling, interior\n"
            "lighting, exterior lighting, interior equipment, exterior equipment,\n"
            "fans, pumps, hot water, and other fuel types. Used to index into\n"
            "the result vectors returned by MonthlyModel and HourlyModel."
        ),
        "EpwData": (
            "EnergyPlus Weather (EPW) file parser and hourly weather data container.",
            "Parses .epw files to extract hourly dry-bulb temperature, wind speed,\n"
            "global horizontal radiation, and other meteorological fields. Computes\n"
            "monthly averages and diurnal profiles for use by the monthly and hourly\n"
            "simulation models."
        ),
        "Heating": (
            "Heating system properties and fuel type parameters.",
            "Stores heating efficiency, temperature setpoints (occupied and unoccupied),\n"
            "HVAC loss and waste factors, pump power, hot water demand, and district\n"
            "heating parameters. Includes fuel type selection for primary energy\n"
            "calculations per ISO 13790."
        ),
        "HourlyModel": (
            "Hourly energy simulation engine using the ISO 13790 hourly method.",
            "Implements the five-resistance-one-capacitance (5R1C) thermal network\n"
            "model from ISO 13790 Annex C. Computes hourly heating, cooling, and\n"
            "electrical energy use over 8760 hours. Includes solar gain, internal\n"
            "gain, ventilation, and HVAC system calculations. Results can be\n"
            "returned hourly or aggregated by month."
        ),
        "ISOModelAPI": (
            "DLL export/import macros for the ISOModel shared library.",
            "Defines the ISOMODEL_API macro for Windows DLL builds. On non-Windows\n"
            "platforms, the macro expands to nothing."
        ),
        "ISOResults": (
            "Simulation result container for monthly energy totals.",
            "Holds the vector of EndUses results returned by MonthlyModel::simulate()\n"
            "and HourlyModel::simulate()."
        ),
        "Lighting": (
            "Lighting system properties and power density parameters.",
            "Stores interior and exterior lighting power densities, dimming and\n"
            "occupancy control fractions, parasitic lighting power, and lighting\n"
            "schedule parameters. Used by MonthlyModel for illumination energy\n"
            "calculations per ISO 15193."
        ),
        "Location": (
            "Site location properties including terrain class and weather data.",
            "Holds the terrain class (urban/suburban/rural wind shielding factor)\n"
            "and a shared pointer to the WeatherData used by the simulation models."
        ),
        "mainpage": (
            "Doxygen main page documentation for the ISOModel library.",
            ""
        ),
        "MathHelpers": (
            "Type aliases (Vector, Matrix) and mathematical utility functions.",
            "Defines Vector (std::vector<double>) and Matrix (std::vector<Vector>)\n"
            "type aliases used throughout the codebase. Provides helper functions\n"
            "for vector arithmetic, element-wise operations, summation, and\n"
            "printing utilities for debug output."
        ),
        "MonthlyModel": (
            "Monthly energy simulation engine using the ISO 13790 monthly method.",
            "Implements the quasi-steady-state monthly energy balance from ISO 13790.\n"
            "Computes monthly heating, cooling, lighting, ventilation, and equipment\n"
            "energy use. Includes envelope heat transfer, solar and internal gains,\n"
            "utilization factors, and HVAC system efficiency calculations."
        ),
        "Population": (
            "Occupancy schedule and people density properties.",
            "Defines occupied/unoccupied hours and days, people density (m²/person),\n"
            "and metabolic heat gain per person. The schedule defines a rectangular\n"
            "occupancy block (hoursStart..hoursEnd × daysStart..daysEnd) used by\n"
            "both monthly and hourly models."
        ),
        "Profiler": (
            "Lightweight scoped function-level profiler for performance analysis.",
            "Provides PROFILE_FUNCTION() and PROFILE_SCOPE() macros that measure\n"
            "wall-clock time using std::chrono. Results are accumulated per-function\n"
            "and printed as a sorted table on destruction. Controlled by the\n"
            "ENABLE_PROFILING compile-time flag."
        ),
        "Schedules": (
            "Hourly and weekly schedule generation and CSV loading.",
            "Builds 24×7 weekly schedule arrays for ventilation, appliances, lighting,\n"
            "and temperature setpoints from Population and Building properties.\n"
            "Optionally loads custom hourly schedules from CSV files. Generates\n"
            "monthly occupancy fractions for the MonthlyModel."
        ),
        "Simulation": (
            "Abstract base class for HourlyModel and MonthlyModel.",
            "Holds shared references to the component property objects (Population,\n"
            "Location, Building, Structure, etc.) that both simulation engines need.\n"
            "Provides the common interface for setting up and running simulations."
        ),
        "SimulationSettings": (
            "ISO 13790 simulation parameters for the 5R1C thermal network.",
            "Stores the internal/solar heat flow distribution fractions (phi_int_is,\n"
            "phi_sol_is) and the surface-to-air heat transfer ratios (h_is, h_ms)\n"
            "used in the hourly 5R1C model. Default values follow ISO 13790 §7.2.2."
        ),
        "SolarRadiation": (
            "Solar position and radiation calculations for building surfaces.",
            "Computes hourly solar radiation on tilted surfaces for all 8760 hours\n"
            "of the year. Implements solar geometry (declination, equation of time,\n"
            "hour angle, altitude, azimuth) per ASHRAE Fundamentals 2013 Ch. 14\n"
            "and Duffie & Beckman. Decomposes global horizontal radiation into\n"
            "beam and diffuse components using the Erbs correlation, then projects\n"
            "onto 9 building surfaces (8 cardinal/intercardinal walls + roof)."
        ),
        "Structure": (
            "Building envelope properties: areas, U-values, SHGC, and thermal mass.",
            "Stores wall/window/roof areas, U-values, solar heat gain coefficients,\n"
            "thermal emissivity, solar absorption, shading device factors, interior\n"
            "and exterior heat capacities, building height, and infiltration rate.\n"
            "Properties are organized as 9-element arrays for the 8 cardinal/\n"
            "intercardinal orientations plus the roof."
        ),
        "TimeFrame": (
            "Hour-of-year to month, day-of-week, and hour-of-day conversion utility.",
            "Pre-computes lookup tables for converting a linear hour index (0–8759)\n"
            "to month (0–11), day of month, day of week (0–6), and hour of day\n"
            "(0–23). Used by HourlyModel and SolarRadiation for time indexing."
        ),
        "UserModel": (
            "High-level facade for loading building models and creating simulations.",
            "Parses ISM (legacy) and YAML configuration files to populate all\n"
            "component property objects (Structure, Heating, Cooling, Ventilation,\n"
            "Lighting, Building, Population, Location, etc.). Provides factory\n"
            "methods to create configured MonthlyModel and HourlyModel instances.\n"
            "Supports optional default values and property overrides."
        ),
        "Ventilation": (
            "Ventilation system properties, infiltration, and heat recovery.",
            "Stores ventilation intake/exhaust rates, heat recovery efficiency,\n"
            "recirculation fraction, fan power, preheat temperature, and air\n"
            "leakage parameters. Includes infiltration model coefficients per\n"
            "ISO 15242 and the overall ventilation heat transfer coefficient\n"
            "(H_ve) per ISO 13790 §9.3."
        ),
        "WeatherData": (
            "Monthly-averaged weather data container for the ISO monthly model.",
            "Stores mean monthly values for global horizontal radiation, dry-bulb\n"
            "temperature, wind speed, and directional solar radiation on vertical\n"
            "surfaces. Also holds diurnal (24-hour) temperature and radiation\n"
            "profiles for each month."
        ),
        "standalone_main": (
            "Command-line entry point for running ISOModel simulations.",
            "Loads a building model from YAML/ISM and EPW files, runs both monthly\n"
            "and hourly simulations, and prints the results to stdout. Demonstrates\n"
            "the UserModel → MonthlyModel/HourlyModel workflow."
        ),
    }

    # Test file summaries
    test_summaries = {
        "HourlyModel_GTest": (
            "Regression tests for the hourly simulation model.",
            "Loads a test building, runs HourlyModel::simulate(), and compares\n"
            "monthly-aggregated results against known-good expected values for\n"
            "all 13 end-use categories across 12 months."
        ),
        "HourlySchedules_GTest": (
            "Regression tests for hourly model with custom CSV schedules.",
            "Same as HourlyModel_GTest but loads custom hourly schedules from\n"
            "a CSV file to verify schedule-driven simulation results."
        ),
        "ISOModel_Benchmark": (
            "Performance benchmarks for hourly and monthly simulations.",
            "Runs multiple iterations of both simulation models with profiling\n"
            "enabled to measure execution time and identify bottlenecks."
        ),
        "ISOModel_GTest": (
            "Top-level Google Test entry point.",
            "Includes the test fixture and serves as the main compilation unit\n"
            "for the test suite."
        ),
        "ISOModelFixture": (
            "Google Test fixture with shared UserModel setup.",
            "Loads the test building YAML and EPW files once in SetUpTestSuite()\n"
            "and provides the configured UserModel to all test cases."
        ),
        "MonthlyModel_GTest": (
            "Regression tests for the monthly simulation model.",
            "Loads a test building, runs MonthlyModel::simulate(), and compares\n"
            "results against known-good expected values for all 13 end-use\n"
            "categories across 12 months."
        ),
        "OptimizationCoverage_GTest": (
            "Tests verifying C++20 optimization and constant correctness.",
            "Validates that constexpr constants have correct values, solar\n"
            "radiation math functions produce expected results, EPW parsing\n"
            "works correctly, and UserModel modern features function properly."
        ),
        "solar_debug": (
            "Debug utility for solar radiation calculations.",
            "Standalone program that loads a building model and prints detailed\n"
            "solar radiation intermediate values for debugging and validation."
        ),
        "SolarRadiation_GTest": (
            "Tests for solar position and radiation calculations.",
            "Validates sun position (altitude, azimuth), surface radiation values,\n"
            "and monthly solar totals against hand-calculated reference data."
        ),
        "TimeFrame_GTest": (
            "Tests for TimeFrame hour/day/month conversion lookups.",
            "Verifies that TimeFrame correctly maps hour-of-year indices to\n"
            "month, day of month, day of week, and hour of day."
        ),
        "UserModel_GTest": (
            "Tests for UserModel property loading and initialization.",
            "Validates that YAML/ISM file parsing correctly populates all\n"
            "component properties, tests default value handling, and verifies\n"
            "optional property override behavior."
        ),
    }

    if base in summaries:
        return summaries[base]
    if base in test_summaries:
        return test_summaries[base]
    return ("TODO: Add description.", "")


def main():
    output_lines = []

    for filepath in SOURCE_FILES:
        git_path = os.path.join("IsoModel", "src", filepath)

        authors = git_log_authors(git_path)
        first_date = git_first_commit_date(git_path)
        existing = get_existing_header(filepath)
        brief, detailed = generate_summary(filepath)

        output_lines.append(f"{'='*72}")
        output_lines.append(f"FILE: {filepath}")
        output_lines.append(f"AUTHORS: {', '.join(authors) if authors else 'Unknown'}")
        output_lines.append(f"CREATED: {first_date}")
        output_lines.append(f"BRIEF: {brief}")
        if detailed:
            output_lines.append(f"DETAILED: {detailed}")
        output_lines.append(f"")
        output_lines.append(f"PROPOSED HEADER:")
        output_lines.append(f"/// @file {os.path.basename(filepath)}")
        output_lines.append(f"/// @brief {brief}")
        if detailed:
            output_lines.append(f"///")
            for detail_line in detailed.split('\n'):
                output_lines.append(f"/// {detail_line}")
        if authors:
            output_lines.append(f"///")
            for author in authors:
                output_lines.append(f"/// @author {author}")
        output_lines.append(f"/// @date {first_date}")
        output_lines.append(f"/// @copyright Copyright Argonne National Laboratory")
        output_lines.append(f"")
        if existing and existing != "(no header)":
            output_lines.append(f"EXISTING HEADER:")
            output_lines.append(existing)
        output_lines.append(f"")

    output_path = os.path.join(SRC_DIR, "file_headers.txt")
    with open(output_path, 'w') as f:
        f.write('\n'.join(output_lines) + '\n')

    print(f"Generated {output_path}")
    print(f"Processed {len(SOURCE_FILES)} files")


if __name__ == "__main__":
    main()
