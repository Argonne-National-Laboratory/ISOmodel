#include "../UserModel.hpp"
#include <iostream>
#include <chrono>

using namespace openstudio::isomodel;

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage: isomodel_unit_tests test_data_directory" << std::endl;
    std::cout << "Running with default directory." << std::endl;

    UserModel userModel;
    userModel.load("../test_data/SmallOffice_v2.ism");
    auto monthlyModel = userModel.toMonthlyModel();
    auto monthlyResults = monthlyModel.simulate();

    std::cout << "Finished simulation" << std::endl;

  } else {
    std::string test_data_path = argv[argc - 1];

    // Set up the hourly and monthly models.
    UserModel userModel;

    std::cout << "loading test data from: " << test_data_path << std::endl;
    userModel.load(test_data_path + "/SmallOffice_v2.ism");

    std::cout << "Creating MonthlyModel" << std::endl;
    MonthlyModel monthlyModel = userModel.toMonthlyModel();

    // Number of iterations to run for each benchmark.
    int iterations = 10000;

    // Benchmark the monthly simulation.
    std::cout << "Benchmark: Running Monthly Simulation. Timing just the simulation. Iterations = " << iterations << std::endl;

    auto monthStart = std::chrono::steady_clock::now();
    for (int i = 0; i != iterations; ++i){
      auto monthlyResults = monthlyModel.simulate();
    }
    auto monthEnd = std::chrono::steady_clock::now();

    auto monthDiff = monthEnd - monthStart;
    double monthlyTime = std::chrono::duration<double, std::micro>(monthDiff).count() / iterations;
    std::cout << "Monthly simulation ran in " << monthlyTime << " us, average over " << iterations << " loops." << std::endl;

    std::cout << "Benchmark: Updating .ism properties with UserModel setters, creating simmodel, running monthly simulation.\n";

    monthStart = std::chrono::steady_clock::now();
    for (int i = 0; i != iterations; ++i){
      // Set the floor, wall and window areas, as if one was doing an orientation
      // optimization study.
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

      auto monthlyModel = userModel.toMonthlyModel();
      auto monthlyResults = monthlyModel.simulate();
    }
    monthEnd = std::chrono::steady_clock::now();

    monthDiff = monthEnd - monthStart;
    monthlyTime = std::chrono::duration<double, std::micro>(monthDiff).count() / iterations;
    std::cout << "Monthly simulation including modifying properties ran in " << monthlyTime << " us, average over " << iterations << " loops." << std::endl;

    std::cout << "Benchmarking monthly simulation with reloading the ism file each run (weather is cached).\n";

    std::cout << "Done!" << std::endl;
  }
}

