/*
 * ISOModel_Benchmark.cpp
 * Refactored to ensure standard C++ compatibility while preserving all original benchmark loops.
 */
#include "../UserModel.hpp"
#include "../MonthlyModel.hpp"
#include "../HourlyModel.hpp"
#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <numeric>

using namespace openstudio::isomodel;

int main(int argc, char** argv)
{
    std::string test_data_path = "../test_data";
    if (argc >= 2) {
        test_data_path = argv[argc - 1];
    }

    UserModel userModel;
    std::cout << "Loading test data from: " << test_data_path << std::endl;

    try {
        // Benchmark logic uses test_bldg.yaml
        userModel.load(test_data_path + "/test_bldg.yaml");
    }
    catch (...) {
        std::cerr << "Failed to load test_bldg.yaml. Ensure path is correct.\n";
        return 1;
    }

    std::cout << "Creating MonthlyModel" << std::endl;
    MonthlyModel monthlyModel = userModel.toMonthlyModel();

    std::cout << "Creating HourlyModel" << std::endl;
    HourlyModel hourlyModel = userModel.toHourlyModel();

    int iterations = 100;
    std::cout << "Benchmark: Running Simulations. Timing just the simulation. Iterations = " << iterations << std::endl;

    // 1. Monthly Benchmark (Static Model)
    auto monthStart = std::chrono::steady_clock::now();
    for (int i = 0; i <= iterations; ++i) {
        auto monthlyResults = monthlyModel.simulate();
    }
    auto monthEnd = std::chrono::steady_clock::now();
    double monthlyTime = std::chrono::duration<double, std::micro>(monthEnd - monthStart).count() / iterations;
    std::cout << "Monthly simulation ran in " << monthlyTime << " us, average over " << iterations << " loops." << std::endl;

    // 2. Hourly Benchmark (Static Model)
    auto hourStart = std::chrono::steady_clock::now();
    for (int i = 0; i <= iterations; ++i) {
        auto hourlyResults = hourlyModel.simulate(true); // aggregateByMonth = true
    }
    auto hourEnd = std::chrono::steady_clock::now();
    double hourlyTime = std::chrono::duration<double, std::micro>(hourEnd - hourStart).count() / iterations;
    std::cout << "Hourly simulation ran in " << hourlyTime << " us, average over " << iterations << " loops." << std::endl;

    // 3. Loop with Property Modification (RESTORING THIS LOOP)
    std::cout << "Benchmark: Updating .ism properties with UserModel setters, creating simmodel, running monthly simulation.\n";
    monthStart = std::chrono::steady_clock::now();
    for (int i = 0; i <= iterations; ++i) {
        // Set the floor, wall and window areas, as if one was doing an orientation optimization study.
        userModel.setFloorArea(511.16);

        userModel.setWallAreaN(84.45);
        userModel.setWallAreaNE(0.0);
        userModel.setWallAreaE(56.3);
        userModel.setWallAreaSE(0.0);
        userModel.setWallAreaS(84.45);
        userModel.setWallAreaSW(0.0);
        userModel.setWallAreaW(56.3);
        userModel.setWallAreaNW(0.0);
        userModel.setRoofArea(598.76);

        userModel.setWindowAreaN(16.74);
        userModel.setWindowAreaNE(0.0);
        userModel.setWindowAreaE(11.16);
        userModel.setWindowAreaSE(0.0);
        userModel.setWindowAreaS(16.74);
        userModel.setWindowAreaSW(0.0);
        userModel.setWindowAreaW(11.16);
        userModel.setWindowAreaNW(0.0);
        userModel.setSkylightArea(0.0);

        // Re-generate model from updated parameters
        auto currentMonthlyModel = userModel.toMonthlyModel();
        auto monthlyResults = currentMonthlyModel.simulate();
    }
    monthEnd = std::chrono::steady_clock::now();

    monthlyTime = std::chrono::duration<double, std::micro>(monthEnd - monthStart).count() / iterations;
    std::cout << "Monthly simulation including modifying properties ran in " << monthlyTime << " us, average over " << iterations << " loops." << std::endl;

    std::cout << "Done!" << std::endl;
    return 0;
}