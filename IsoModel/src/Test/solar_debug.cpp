/*
 * solar_debug.cpp
 *
 * Refactored to remove boost dependencies and ensure output prints regardless of global debug flags.
 */

#include "../UserModel.hpp"
#include "../MonthlyModel.hpp"
#include "../SolarRadiation.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

using namespace openstudio::isomodel;
using namespace openstudio;

void printUsage(const char* execName) {
    std::cout << "Usage: " << execName << " <ismfilepath> [options]\n"
        << "Options:\n"
        << "  -i, --ismfilepath <path>  Path to .ism file.\n"
        << "  -m, --monthly             Print the monthly solar radiation values.\n"
        << "  -h, --hourly              Print the hourly solar radiation values.\n";
}

// Explicit printing to bypass global DEBUG_ISO_MODEL_SIMULATION restrictions
void forcePrintMonthlySolar(UserModel umodel) {
    umodel.loadWeather();
    WeatherData wd = *umodel.weatherData();
    Matrix msolar = wd.msolar();
    Matrix mhEgh = wd.mhEgh();
    Vector mEgh = wd.mEgh();

    std::cout << "\n--- Monthly Solar Radiation (msolar) ---" << std::endl;
    std::cout << "Month, S, SE, E, NE, N, NW, W, SW" << std::endl;
    for (size_t i = 0; i < msolar.size1(); ++i) {
        std::cout << i + 1;
        for (size_t j = 0; j < msolar.size2(); ++j) {
            std::cout << ", " << std::fixed << std::setprecision(4) << msolar(i, j);
        }
        std::cout << std::endl;
    }

    std::cout << "\n--- Monthly Global Horizontal Radiation (mEgh) ---" << std::endl;
    std::cout << "Month, W/m2" << std::endl;
    for (size_t i = 0; i < mEgh.size(); ++i) {
        std::cout << i + 1 << ", " << mEgh[i] << std::endl;
    }

    std::cout << "\n--- Monthly Hourly Average Egh (mhEgh) ---" << std::endl;
    for (size_t i = 0; i < mhEgh.size1(); ++i) {
        std::cout << "Month " << i + 1;
        for (size_t j = 0; j < mhEgh.size2(); ++j) {
            std::cout << ", " << mhEgh(i, j);
        }
        std::cout << std::endl;
    }
}

void forcePrintHourlySolar(UserModel umodel) {
    umodel.loadWeather();

    TimeFrame frame;
    SolarRadiation pos(&frame, umodel.epwData().get());
    pos.Calculate();
    std::vector<std::vector<double>> radiation = pos.eglobe();

    // Add the roof radiation (9th direction). EGH is global horizontal radiation.
    for (size_t i = 0; i < radiation.size(); ++i) {
        radiation[i].push_back(umodel.epwData()->data()[EGH][i]);
    }

    std::cout << "\n--- Hourly Solar Radiation ---" << std::endl;
    std::cout << "Hour, S, SE, E, NE, N, NW, W, SW, Horizontal" << std::endl;

    for (size_t i = 0; i < radiation.size(); ++i) {
        std::cout << i;
        for (const auto& val : radiation[i]) {
            std::cout << ", " << std::fixed << std::setprecision(4) << val;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string ismPath;
    bool runMonthly = false;
    bool runHourly = false;

    // Improved argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--ismfilepath") {
            if (++i < argc) ismPath = argv[i];
        }
        else if (arg == "-m" || arg == "--monthly") {
            runMonthly = true;
        }
        else if (arg == "-h" || arg == "--hourly") {
            runHourly = true;
        }
        else if (arg[0] != '-') {
            ismPath = arg;
        }
    }

    if (ismPath.empty()) {
        std::cerr << "ERROR: ismfilepath is required.\n";
        printUsage(argv[0]);
        return 1;
    }

    openstudio::isomodel::UserModel umodel;
    try {
        umodel.load(ismPath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        return 1;
    }

    // Logic: Default to monthly if no flags are set, or run requested flags
    if (!runMonthly && !runHourly) {
        runMonthly = true;
    }

    if (runMonthly) {
        forcePrintMonthlySolar(umodel);
    }

    if (runHourly) {
        forcePrintHourlySolar(umodel);
    }

    return 0;
}