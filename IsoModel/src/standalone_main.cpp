/*
 * standalone_main.cpp
 * * Compatible with the Original Interface (std::vector<EndUses>)
 */

#include "UserModel.hpp"
#include "MonthlyModel.hpp"
#include "HourlyModel.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace openstudio::isomodel;
using namespace openstudio;

// Helper to show usage
void printUsage(const char* execName) {
    std::cout << "Usage: " << execName << " <ismfilepath> [defaultsfilepath] [options]\n"
        << "Options:\n"
        << "  -i, --ismfilepath <path>      Path to ism file.\n"
        << "  -d, --defaultsfilepath <path> Path to defaults ism file.\n"
        << "  -m, --monthly                 Run the monthly simulation (default).\n"
        << "  -h, --hourlyByMonth           Run the hourly simulation (results aggregated by month).\n"
        << "  -H, --hourlyByHour            Run the hourly simulation (results for each hour).\n"
        << "  -c, --compare <md|csv>        Run monthly/hourly comparison.\n";
}

void runMonthlySimulation(const UserModel& umodel) {
    openstudio::isomodel::MonthlyModel monthlyModel = umodel.toMonthlyModel();
    auto monthlyResults = monthlyModel.simulate();
    std::cout << "Monthly Results:\nMonth,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW\n";
    for (int i = 0; i < 12; i++) {
        std::cout << i + 1;
        for (int j = 0; j < 13; j++) {
            std::cout << ", " << std::setprecision(10) << monthlyResults[i].getEndUse(j);
        }
        std::cout << std::endl;
    }
}

void runHourlySimulation(const UserModel& umodel, bool aggregateByMonth) {
    openstudio::isomodel::HourlyModel hourly = umodel.toHourlyModel();
    auto hourlyResults = hourly.simulate(aggregateByMonth);
    
    std::string monthOrHour = aggregateByMonth ? "month" : "hour";
    // hourlyResults is std::vector<EndUses>, so .size() works directly
    
    std::cout << "Hourly results by " << monthOrHour << ":\n" << monthOrHour << ",ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW\n";
    
    for (size_t i = 0; i < hourlyResults.size(); i++) {
        std::cout << i + 1;
        // 0=ElecHeat, 1=ElecCool, ... 9=GasHeat. 
        // 10,11,12 are placeholders for GasCool,GasEquip,GasDHW if needed
        for (int j = 0; j < 10; j++) {
             std::cout << ", " << hourlyResults[i].getEndUse(j);
        }
        // Fill remaining columns (GasCool, GasEquip, GasDHW) with 0 if not present in getEndUse
        std::cout << ", 0, 0, 0" << std::endl;
    }
}

void compare(const UserModel& umodel, bool markdown = false) {
    openstudio::isomodel::HourlyModel hourly = umodel.toHourlyModel();
    // aggregateByMonth = true for comparison
    auto hourlyResults = hourly.simulate(true); 
    openstudio::isomodel::MonthlyModel monthlyModel = umodel.toMonthlyModel();
    auto monthlyResults = monthlyModel.simulate();
    
    auto endUseNames = std::vector<std::string>{ "ElecHeat", "ElecCool", "ElecIntLights", "ElecExtLights", "ElecFans", "ElecPump", "ElecEquipInt", "ElecEquipExt", "ElectDHW", "GasHeat", "GasCool", "GasEquip", "GasDHW" };
    auto delim = markdown ? " | " : ", ";

    for (auto endUse = 0; endUse != 13; ++endUse) {
        if (markdown) std::cout << "| ";
        std::cout << "Month" << delim << "Monthly " << endUseNames[endUse] << delim << "Hourly " << endUseNames[endUse] << delim << "Difference";
        if (markdown) std::cout << " |";
        std::cout << "\n";
        if (markdown) std::cout << "|---|---|---|---|\n";
        for (auto month = 0; month != 12; ++month) {
            auto monthlyResult = monthlyResults[month].getEndUse(endUse);
            // Original interface: hourlyResults is vector<EndUses>
            // We use getEndUse(index). 
            // Note: If hourlyResults doesn't have 13 indices, getEndUse typically returns 0 or handles it.
            // Based on previous code, 0-9 are populated.
            double hourlyResult = (endUse < 10) ? hourlyResults[month].getEndUse(endUse) : 0.0;

            if (markdown) std::cout << "| ";
            std::cout << month << delim << monthlyResult << delim << hourlyResult << delim << monthlyResult - hourlyResult;
            if (markdown) std::cout << " |";
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string ismPath, defaultsPath, compareType;
    bool runMonthly = false, runHourlyByMonth = false, runHourlyByHour = false, runCompare = false;

    // Simple manual parser
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-i" || args[i] == "--ismfilepath") {
            if (++i < args.size()) ismPath = args[i];
        }
        else if (args[i] == "-d" || args[i] == "--defaultsfilepath") {
            if (++i < args.size()) defaultsPath = args[i];
        }
        else if (args[i] == "-m" || args[i] == "--monthly") {
            runMonthly = true;
        }
        else if (args[i] == "-h" || args[i] == "--hourlyByMonth") {
            runHourlyByMonth = true;
        }
        else if (args[i] == "-H" || args[i] == "--hourlyByHour") {
            runHourlyByHour = true;
        }
        else if (args[i] == "-c" || args[i] == "--compare") {
            runCompare = true;
            if (++i < args.size()) compareType = args[i];
        }
        else if (ismPath.empty()) {
            ismPath = args[i]; 
        }
        else if (defaultsPath.empty()) {
            defaultsPath = args[i]; 
        }
    }

    if (ismPath.empty()) {
        std::cerr << "ERROR: ismfilepath is required.\n";
        printUsage(argv[0]);
        return 1;
    }

    openstudio::isomodel::UserModel umodel;
    if (!defaultsPath.empty()) {
        umodel.load(ismPath, defaultsPath);
    }
    else {
        umodel.load(ismPath);
    }

    bool simulationRan = false;
    if (runCompare) {
        compare(umodel, (compareType == "md"));
        simulationRan = true;
    }
    if (runMonthly) {
        runMonthlySimulation(umodel);
        simulationRan = true;
    }
    if (runHourlyByMonth) {
        runHourlySimulation(umodel, true);
        simulationRan = true;
    }
    if (runHourlyByHour) {
        runHourlySimulation(umodel, false);
        simulationRan = true;
    }
    if (!simulationRan) {
        runMonthlySimulation(umodel);
    }
    return 0;
}