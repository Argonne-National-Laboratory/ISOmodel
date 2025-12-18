/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#include "UserModel.hpp"
#include <optional>
#include <filesystem>

using namespace std;
namespace openstudio {
    namespace isomodel {

        UserModel::UserModel() :
            _weather_cache(), _weather(new WeatherData()), _edata(new EpwData())
        {
        }

        UserModel::~UserModel()
        {
        }

        void UserModel::setCoreSimulationProperties(Simulation& sim) const {
            sim.setPop(pop);
            sim.setBuilding(building);
            sim.setCooling(cooling);
            sim.setHeating(heating);
            sim.setLights(lights);
            sim.setStructure(structure);
            sim.setVentilation(ventilation);
            sim.setLocation(location);
            sim.setEpwData(_edata); // TODO: should this stay a shared pointer between the UserModel and the MonthlyModel?
            sim.setSimulationSettings(simSettings);
            sim.setPhysicalQuantities(phys);
        }

        HourlyModel UserModel::toHourlyModel() const
        {
            HourlyModel sim = HourlyModel();
            if (!_valid) {
                return *((HourlyModel*)NULL);
            }

            setCoreSimulationProperties(sim);
            return sim;
        }

        MonthlyModel UserModel::toMonthlyModel() const
        {

            MonthlyModel sim;

            if (!valid()) {
                std::cout << "Invalid" << std::endl;
                return *((MonthlyModel*)NULL);
            }

            setCoreSimulationProperties(sim);

            return sim;
        }
        //http://stackoverflow.com/questions/10051679/c-tokenize-string
        std::vector<std::string> inline stringSplit(const std::string& source, char delimiter = ' ', bool keepEmpty = false)
        {
            std::vector<std::string> results;

            size_t prev = 0;
            size_t next = 0;
            if (source.size() == 0)
                return results;
            while ((next = source.find_first_of(delimiter, prev)) != std::string::npos) {
                if (keepEmpty || (next - prev != 0)) {
                    results.push_back(source.substr(prev, next - prev));
                }
                prev = next + 1;
            }

            if (prev < source.size()) {
                results.push_back(source.substr(prev));
            }

            return results;
        }

        void UserModel::initializeStructure(const YAML::Node& buildingParams)
        {
            initializeParameter(&UserModel::setWallArea, buildingParams, "wallArea", true);
            initializeParameter(&UserModel::setWallU, buildingParams, "wallU", true);
            initializeParameter(&UserModel::setWallThermalEmissivity, buildingParams, "wallEmissivity", true);
            initializeParameter(&UserModel::setWallSolarAbsorption, buildingParams, "wallAbsorption", true);
            initializeParameter(&UserModel::setWindowArea, buildingParams, "windowArea", true);
            initializeParameter(&UserModel::setWindowU, buildingParams, "windowU", true);
            initializeParameter(&UserModel::setWindowSHGC, buildingParams, "windowSHGC", true);
            initializeParameter(&UserModel::setWindowSCF, buildingParams, "windowSCF", true);
            initializeParameter(&UserModel::setWindowSDF, buildingParams, "windowSDF", true);
        }

        void UserModel::initializeParameters(const YAML::Node& buildingParams)
        {
            initializeParameter(&UserModel::setTerrainClass, buildingParams, "terrainclass", true);
            initializeParameter(&UserModel::setBuildingHeight, buildingParams, "buildingheight", true);
            initializeParameter(&UserModel::setFloorArea, buildingParams, "floorarea", true);
            initializeParameter(&UserModel::setBuildingOccupancyFrom, buildingParams, "occupancydayfirst", true);
            initializeParameter(&UserModel::setBuildingOccupancyTo, buildingParams, "occupancydaylast", true);
            initializeParameter(&UserModel::setEquivFullLoadOccupancyFrom, buildingParams, "occupancyhourfirst", true);
            initializeParameter(&UserModel::setEquivFullLoadOccupancyTo, buildingParams, "occupancyhourlast", true);
            initializeParameter(&UserModel::setPeopleDensityOccupied, buildingParams, "peopledensityoccupied", true);
            initializeParameter(&UserModel::setPeopleDensityUnoccupied, buildingParams, "peopledensityunoccupied", true);
            initializeParameter(&UserModel::setLightingPowerIntensityOccupied, buildingParams, "lightingpowerdensityoccupied", true);
            initializeParameter(&UserModel::setLightingPowerIntensityUnoccupied, buildingParams, "lightingpowerdensityunoccupied", true);
            initializeParameter(&UserModel::setElecPowerAppliancesOccupied, buildingParams, "electricappliancepowerdensityoccupied", true);
            initializeParameter(&UserModel::setElecPowerAppliancesUnoccupied, buildingParams, "electricappliancepowerdensityunoccupied", true);
            initializeParameter(&UserModel::setGasPowerAppliancesOccupied, buildingParams, "gasappliancepowerdensityoccupied", true);
            initializeParameter(&UserModel::setGasPowerAppliancesUnoccupied, buildingParams, "gasappliancepowerdensityunoccupied", true);
            initializeParameter(&UserModel::setExteriorLightingPower, buildingParams, "exteriorlightingpower", true);
            initializeParameter(&UserModel::setHvacWasteFactor, buildingParams, "hvacwastefactor", true);
            initializeParameter(&UserModel::setHvacHeatingLossFactor, buildingParams, "hvacheatinglossfactor", true);
            initializeParameter(&UserModel::setHvacCoolingLossFactor, buildingParams, "hvaccoolinglossfactor", true);
            initializeParameter(&UserModel::setDaylightSensorSystem, buildingParams, "daylightsensordimmingfraction", true);
            initializeParameter(&UserModel::setLightingOccupancySensorSystem, buildingParams, "lightingoccupancysensordimmingfraction", true);
            initializeParameter(&UserModel::setConstantIlluminationControl, buildingParams, "constantilluminationcontrolmultiplier", true);
            initializeParameter(&UserModel::setCoolingSystemCOP, buildingParams, "coolingsystemcop", true);
            initializeParameter(&UserModel::setCoolingSystemIPLVToCOPRatio, buildingParams, "coolingsystemiplvtocopratio", true);

            initializeParameter(&UserModel::setHeatingSystemEfficiency, buildingParams, "heatingsystemefficiency", true);

            // Create the named function pointers to disambiguate which overload of setter we want.
            // TODO: Switch the keyword based properties use enums throughout, rather than the current
            // combination of strings and doubles. If the setter has a consistent interface and doesn't
            // have to be overloaded, we don't have to do this disambiguation. BAA@2015-06-24
            void(UserModel:: * setHeatingEnergyCarrierWithString)(std::string) = &UserModel::setHeatingEnergyCarrier;
            initializeParameter(setHeatingEnergyCarrierWithString, buildingParams, "heatingfueltype", true);

            void(UserModel:: * setVentilationTypeWithString)(std::string) = &UserModel::setVentilationType;
            initializeParameter(setVentilationTypeWithString, buildingParams, "ventilationtype", true);

            void(UserModel:: * setDhwEnergyCarrierWithString)(std::string) = &UserModel::setDhwEnergyCarrier;
            initializeParameter(setDhwEnergyCarrierWithString, buildingParams, "dhwfueltype", true);

            void(UserModel:: * setBemTypeWithString)(std::string) = &UserModel::setBemType;
            initializeParameter(setBemTypeWithString, buildingParams, "bemtype", true);

            initializeParameter(&UserModel::setFreshAirFlowRate, buildingParams, "ventilationintakerateoccupied", true);
            initializeParameter(&UserModel::setSupplyExhaustRate, buildingParams, "ventilationExhaustRateOccupied", true);
            initializeParameter(&UserModel::setHeatRecovery, buildingParams, "heatrecovery", true);
            initializeParameter(&UserModel::setExhaustAirRecirclation, buildingParams, "exhaustairrecirculation", true);
            initializeParameter(&UserModel::setBuildingAirLeakage, buildingParams, "infiltrationrateoccupied", true);
            initializeParameter(&UserModel::setDhwDemand, buildingParams, "dhwdemand", true);
            initializeParameter(&UserModel::setDhwEfficiency, buildingParams, "dhwsystemefficiency", true);
            initializeParameter(&UserModel::setDhwDistributionEfficiency, buildingParams, "dhwdistributionefficiency", true);

            initializeParameter(&UserModel::setInteriorHeatCapacity, buildingParams, "interiorheatcapacity", true);
            initializeParameter(&UserModel::setExteriorHeatCapacity, buildingParams, "exteriorheatcapacity", true);
            initializeParameter(&UserModel::setHeatingPumpControl, buildingParams, "heatingpumpcontrol", true);
            initializeParameter(&UserModel::setCoolingPumpControl, buildingParams, "coolingpumpcontrol", true);
            initializeParameter(&UserModel::setHeatGainPerPerson, buildingParams, "heatgainperperson", true);
            initializeParameter(&UserModel::setSpecificFanPower, buildingParams, "specificfanpower", true);
            initializeParameter(&UserModel::setFanFlowControlFactor, buildingParams, "fanflowcontrolfactor", true);
            initializeParameter(&UserModel::setCoolingOccupiedSetpoint, buildingParams, "coolingsetpointoccupied", true);
            initializeParameter(&UserModel::setCoolingUnoccupiedSetpoint, buildingParams, "coolingsetpointunoccupied", true);
            initializeParameter(&UserModel::setHeatingOccupiedSetpoint, buildingParams, "heatingsetpointoccupied", true);
            initializeParameter(&UserModel::setHeatingUnoccupiedSetpoint, buildingParams, "heatingsetpointunoccupied", true);

#if (USE_NEW_BUILDING_PARAMS)
            initializeParameter(&UserModel::setVentilationIntakeRateUnoccupied, buildingParams, "ventilationIntakeRateUnoccupied", true);
            initializeParameter(&UserModel::setVentilationExhaustRateUnoccupied, buildingParams, "ventilationExhaustRateUnoccupied", true);
            initializeParameter(&UserModel::setInfiltrationRateUnoccupied, buildingParams, "infiltrationRateUnoccupied", true);
            initializeParameter(&UserModel::setLightingPowerFixedOccupied, buildingParams, "lightingPowerFixedOccupied", true);
            initializeParameter(&UserModel::setLightingPowerFixedUnoccupied, buildingParams, "lightingPowerFixedUnoccupied", true);
            initializeParameter(&UserModel::setElectricAppliancePowerFixedOccupied, buildingParams, "electricAppliancePowerFixedOccupied", true);
            initializeParameter(&UserModel::setElectricAppliancePowerFixedUnoccupied, buildingParams, "electricAppliancePowerFixedUnoccupied", true);
            initializeParameter(&UserModel::setGasAppliancePowerFixedOccupied, buildingParams, "gasAppliancePowerFixedOccupied", true);
            initializeParameter(&UserModel::setGasAppliancePowerFixedUnoccupied, buildingParams, "gasAppliancePowerFixedUnoccupied", true);

            initializeParameter(&UserModel::setScheduleFilePath, buildingParams, "schedulefilepath", true);
#endif

            initializeParameter(&UserModel::setWeatherFilePath, buildingParams, "weatherfilepath", true);

            // Optional properties with hard-coded default values:
            initializeParameter(&UserModel::setExternalEquipment, buildingParams, "externalequipment", false);
            initializeParameter(&UserModel::setForcedAirCooling, buildingParams, "forcedaircooling", false);
            initializeParameter(&UserModel::setT_cl_ctrl_flag, buildingParams, "t_cl_ctrl_flag", false);
            initializeParameter(&UserModel::setDT_supp_cl, buildingParams, "dt_supp_cl", false);
            initializeParameter(&UserModel::setDC_YesNo, buildingParams, "dc_yesno", false);
            initializeParameter(&UserModel::setEta_DC_network, buildingParams, "eta_dc_network", false);
            initializeParameter(&UserModel::setEta_DC_COP, buildingParams, "eta_dc_cop", false);
            initializeParameter(&UserModel::setEta_DC_frac_abs, buildingParams, "eta_dc_frac_abs", false);
            initializeParameter(&UserModel::setEta_DC_COP_abs, buildingParams, "eta_dc_cop_abs", false);
            initializeParameter(&UserModel::setFrac_DC_free, buildingParams, "frac_dc_free", false);
            initializeParameter(&UserModel::setE_pumps_cl, buildingParams, "e_pumps_cl", false);
            initializeParameter(&UserModel::setForcedAirHeating, buildingParams, "forcedairheating", false);
            initializeParameter(&UserModel::setDT_supp_ht, buildingParams, "dt_supp_ht", false);
            initializeParameter(&UserModel::setE_pumps_ht, buildingParams, "e_pumps_ht", false);
            initializeParameter(&UserModel::setT_ht_ctrl_flag, buildingParams, "t_ht_ctrl_flag", false);
            initializeParameter(&UserModel::setA_H0, buildingParams, "a_h0", false);
            initializeParameter(&UserModel::setTau_H0, buildingParams, "tau_h0", false);
            initializeParameter(&UserModel::setDH_YesNo, buildingParams, "dh_yesno", false);
            initializeParameter(&UserModel::setEta_DH_network, buildingParams, "eta_dh_network", false);
            initializeParameter(&UserModel::setEta_DH_sys, buildingParams, "eta_dh_sys", false);
            initializeParameter(&UserModel::setFrac_DH_free, buildingParams, "frac_dh_free", false);
            initializeParameter(&UserModel::setDhw_tset, buildingParams, "dhw_tset", false);
            initializeParameter(&UserModel::setDhw_tsupply, buildingParams, "dhw_tsupply", false);
            initializeParameter(&UserModel::setN_day_start, buildingParams, "n_day_start", false);
            initializeParameter(&UserModel::setN_day_end, buildingParams, "n_day_end", false);
            initializeParameter(&UserModel::setN_weeks, buildingParams, "n_weeks", false);
            initializeParameter(&UserModel::setElecInternalGains, buildingParams, "elecinternalgains", false);
            initializeParameter(&UserModel::setPermLightPowerDensity, buildingParams, "permlightpowerdensity", false);
            initializeParameter(&UserModel::setPresenceSensorAd, buildingParams, "presencesensorad", false);
            initializeParameter(&UserModel::setAutomaticAd, buildingParams, "automaticad", false);
            initializeParameter(&UserModel::setPresenceAutoAd, buildingParams, "presenceautoad", false);
            initializeParameter(&UserModel::setManualSwitchAd, buildingParams, "manualswitchad", false);
            initializeParameter(&UserModel::setPresenceSensorLux, buildingParams, "presencesensorlux", false);
            initializeParameter(&UserModel::setAutomaticLux, buildingParams, "automaticlux", false);
            initializeParameter(&UserModel::setPresenceAutoLux, buildingParams, "presenceautolux", false);
            initializeParameter(&UserModel::setManualSwitchLux, buildingParams, "manualswitchlux", false);
            initializeParameter(&UserModel::setNaturallyLightedArea, buildingParams, "naturallylightedarea", false);
            initializeParameter(&UserModel::setRhoCpAir, buildingParams, "rhocpair", false);
            initializeParameter(&UserModel::setRhoCpWater, buildingParams, "rhocpwater", false);
            initializeParameter(&UserModel::setPhiIntFractionToAirNode, buildingParams, "phiintfractiontoairnode", false);
            initializeParameter(&UserModel::setPhiSolFractionToAirNode, buildingParams, "phisolfractiontoairnode", false);
            initializeParameter(&UserModel::setHci, buildingParams, "hci", false);
            initializeParameter(&UserModel::setHri, buildingParams, "hri", false);
            initializeParameter(&UserModel::setR_se, buildingParams, "r_se", false);
            initializeParameter(&UserModel::setIrradianceForMaxShadingUse, buildingParams, "irradianceformaxshadinguse", false);
            initializeParameter(&UserModel::setShadingFactorAtMaxUse, buildingParams, "shadingfactoratmaxuse", false);
            initializeParameter(&UserModel::setTotalAreaPerFloorArea, buildingParams, "totalareaperfloorarea", false);
            initializeParameter(&UserModel::setWin_ff, buildingParams, "win_ff", false);
            initializeParameter(&UserModel::setWin_F_W, buildingParams, "win_f_w", false);
            initializeParameter(&UserModel::setR_sc_ext, buildingParams, "r_sc_ext", false);
            initializeParameter(&UserModel::setVentPreheatDegC, buildingParams, "ventpreheatdegc", false);
            initializeParameter(&UserModel::setN50, buildingParams, "n50", false);
            initializeParameter(&UserModel::setHzone, buildingParams, "hzone", false);
            initializeParameter(&UserModel::setP_exp, buildingParams, "p_exp", false);
            initializeParameter(&UserModel::setZone_frac, buildingParams, "zone_frac", false);
            initializeParameter(&UserModel::setStack_exp, buildingParams, "stack_exp", false);
            initializeParameter(&UserModel::setStack_coeff, buildingParams, "stack_coeff", false);
            initializeParameter(&UserModel::setWind_exp, buildingParams, "wind_exp", false);
            initializeParameter(&UserModel::setWind_coeff, buildingParams, "wind_coeff", false);
            initializeParameter(&UserModel::setDCp, buildingParams, "dcp", false);
            initializeParameter(&UserModel::setVent_rate_flag, buildingParams, "vent_rate_flag", false);
            initializeParameter(&UserModel::setH_ve, buildingParams, "h_ve", false);
        }

        template <typename T>
        std::optional<T> getParameter(const YAML::Node& params,
            const std::string& paramName) {
            std::string pn(paramName);
            std::transform(paramName.begin(), paramName.end(), pn.begin(), ::tolower);
            if (params[pn]) {
                try {
                    return params[pn].as<T>();
                }
                catch (YAML::TypedBadConversion<T>& ex) {
                    return std::nullopt;
                }
            }
            return std::nullopt;
        }

        bool getParameterAsVector(const YAML::Node& params,
            const std::string& paramName, Vector& vec) {

            std::string pn(paramName);
            std::transform(paramName.begin(), paramName.end(), pn.begin(), ::tolower);
            if (params[pn]) {
                vec.clear();
                auto param = params[pn];
                size_t n = std::distance(param.begin(), param.end());
                if (vec.size() != n) {
                    vec.resize(n);
                }
                // Assign via iterators won't work as they aren't typed to double
                try {
                    size_t index = 0;
                    for (const auto& v : param) {
                        vec[index] = v.as<double>();
                        ++index;
                    }
                    return true;
                }
                catch (YAML::TypedBadConversion<double>& ex) {
                    return false;
                }
            }
            return false;
        }


        void UserModel::initializeParameter(void(UserModel::* setProp)(double), const YAML::Node& params, std::string paramName, bool required) {
            if (auto prop = getParameter<double>(params, paramName)) {
                (this->*setProp)(*prop);
            }
            else if (required) {
                throw std::invalid_argument("Required property " + paramName + " missing in .ism file.");
            }
        }

        void UserModel::initializeParameter(void(UserModel::* setProp)(int), const YAML::Node& params, std::string paramName, bool required) {
            if (auto prop = getParameter<int>(params, paramName)) {
                (this->*setProp)(*prop);
            }
            else if (required) {
                throw std::invalid_argument("Required property " + paramName + " missing in .ism file.");
            }
        }

        void UserModel::initializeParameter(void(UserModel::* setProp)(bool), const YAML::Node& params, std::string paramName, bool required) {
            if (auto prop = getParameter<bool>(params, paramName)) {
                (this->*setProp)(*prop);
            }
            else if (required) {
                throw std::invalid_argument("Required property " + paramName + " missing in .ism file.");
            }
        }

        void UserModel::initializeParameter(void(UserModel::* setProp)(const Vector&), const YAML::Node& params, std::string paramName, bool required) {
            Vector vec;
            if (getParameterAsVector(params, paramName, vec)) {
                // TODO: Update the .ism format order to match the order used internall so we don't
                // have to do this reordering. BAA@2015-06-24.
                northToSouth(vec);
                (this->*setProp)(vec);
            }
            else if (required) {
                throw std::invalid_argument("Required property " + paramName + " missing in .ism file.");
            }
        }

        void UserModel::initializeParameter(void(UserModel::* setProp)(std::string), const YAML::Node& params, std::string paramName, bool required) {
            if (auto prop = getParameter<std::string>(params, paramName)) {
                (this->*setProp)(*prop);
            }
            else if (required) {
                throw std::invalid_argument("Required property " + paramName + " missing in .ism file.");
            }
        }

        void UserModel::northToSouth(Vector& vec) {
            // .ism file is N, NE, E, SE, S, SW, W, NW, Roof
            // Structure is S, SE, E, NE, N, NW, W, SW, Roof
            double temp;

            // Swap 0 and 4 (N and S).
            temp = vec[0];
            vec[0] = vec[4];
            vec[4] = temp;

            // Swap 1 and 3 (NE and SE).
            temp = vec[1];
            vec[1] = vec[3];
            vec[3] = temp;

            // Swap 5 and 7 (SW and NW).
            temp = vec[5];
            vec[5] = vec[7];
            vec[7] = temp;
        };

        void UserModel::loadBuilding(std::string buildingFile)
        {
            YAML::Node tmp = YAML::LoadFile(buildingFile);
            YAML::Node buildingParams = YAML::Load("{}");

            for (auto iter : tmp) {
                std::string key = iter.first.as<std::string>();
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                buildingParams[key] = iter.second;
            }
            size_t n_param = std::distance(buildingParams.begin(), buildingParams.end());
            if (n_param == 0) {
                throw std::invalid_argument("No parameters found in building file " + buildingFile + ". Is this a YAML format file?");
            }
            initializeParameters(buildingParams);
            initializeStructure(buildingParams);
        }

        void UserModel::loadBuilding(std::string buildingFile, std::string defaultsFile)
        {
            YAML::Node bf = YAML::LoadFile(buildingFile);
            size_t n_param = std::distance(bf.begin(), bf.end());
            if (n_param == 0) {
                throw std::invalid_argument("No parameters found in building file " + buildingFile + ". Is this a YAML format file?");
            }

            YAML::Node df = YAML::LoadFile(defaultsFile);
            n_param = std::distance(df.begin(), df.end());
            if (n_param == 0) {
                throw std::invalid_argument("No parameters found in building file " + defaultsFile + ". Is this a YAML format file?");
            }

            YAML::Node buildingParams = YAML::Load("{}");
            for (auto iter : df) {
                std::string key = iter.first.as<std::string>();
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                buildingParams[key] = iter.second;
            }

            for (auto iter : bf) {
                std::string key = iter.first.as<std::string>();
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                buildingParams[key] = iter.second;
            }

            initializeParameters(buildingParams);
            initializeStructure(buildingParams);
        }

        int UserModel::weatherState(std::string header)
        {
            if (!header.compare("solar"))
                return 1;
            else if (!header.compare("hdbt"))
                return 2;
            else if (!header.compare("hEgh"))
                return 3;
            else if (!header.compare("mEgh"))
                return 4;
            else if (!header.compare("mdbt"))
                return 5;
            else if (!header.compare("mwind"))
                return 6;
            else
                return -1;
        }
        std::string UserModel::resolveFilename(std::string baseFile, std::string relativeFile)
        {
            unsigned int lastSeparator = 0;
            unsigned int i = 0;
            const char separatorChar = '/';
            const char winSeparatorChar = '\\';
            std::string result;
            for (; i < baseFile.length(); i++) {
                result += (baseFile[i] == winSeparatorChar) ? separatorChar : baseFile[i];
                if (result[i] == separatorChar) {
                    lastSeparator = i;
                }
            }
            result = result.substr(0, lastSeparator + 1);
            unsigned int j = 0;
            if (relativeFile.length() > 0) {
                //if first char is a separator, skip it
                if (relativeFile[0] == separatorChar || relativeFile[0] == winSeparatorChar)
                    j++;
            }
            for (; j < relativeFile.length(); j++, i++) {
                result += (relativeFile[j] == winSeparatorChar) ? separatorChar : relativeFile[j];
            }
            return result;
        }

        void UserModel::loadWeather()
        {
            std::string weatherFilename;
            //see if weather file path is absolute path
            //if so, use it, else assemble relative path
            if (std::filesystem::exists(_weatherFilePath)) {
                weatherFilename = _weatherFilePath;
            }
            else {
                weatherFilename = resolveFilename(dataFile, _weatherFilePath);
                if (!std::filesystem::exists(weatherFilename)) {
                    std::cout << "Weather File Not Found: " << _weatherFilePath << std::endl;
                    _valid = false;
                }
            }

            _edata->loadData(weatherFilename);
            initializeSolar();
            location.setWeatherData(_weather);
        }

        void UserModel::loadAndSetWeather()
        {
            loadWeather();
            _valid = true;
        }

        bool LatLon::operator <(const LatLon& rhs) const {
            if (lat < rhs.lat) return true;
            if (lat > rhs.lat) return false;

            if (lon < rhs.lon) return true;
            return false;
        }

        void UserModel::loadWeather(int block_size, double* weather_data)
        {

            double lat = weather_data[0];
            double lon = weather_data[1];

            LatLon latlon = { lat, lon };
            auto iter = _weather_cache.find(latlon);
            if (iter == _weather_cache.end()) {
                //std::cout << "not in cache" << std::endl;
                _weather = make_shared<WeatherData>();
                _weather_cache.insert(make_pair(latlon, _weather));
                _edata->loadData(block_size, weather_data);
                initializeSolar();
            }
            else {
                //std::cout << "in cache" << std::endl;
                _weather = iter->second;
            }

            location.setWeatherData(_weather);

            _valid = true;
        }

        void UserModel::initializeSolar()
        {

            int state = 0, row = 0;
            Matrix _msolar(12, 8);
            Matrix _mhdbt(12, 24);
            Matrix _mhEgh(12, 24);
            Vector _mEgh(12);
            Vector _mdbt(12);
            Vector _mwind(12);

            string line;
            std::vector<std::string> linesplit;

            std::stringstream inputFile(_edata->toISOData());

            while (inputFile.good()) {
                getline(inputFile, line);
                if (line.size() > 0 && line[0] == '#')
                    continue;
                linesplit = stringSplit(line, ',', true);
                if (linesplit.size() == 0) {
                    continue;
                }
                else if (linesplit.size() == 1) {
                    state = weatherState(linesplit[0]);
                    row = 0;
                }
                else if (row < 12) {
                    switch (state) {
                    case 1: //solar = [12 x 8] mean monthly total solar radiation (W/m2) on a vertical surface for each of the 8 cardinal directions
                        for (unsigned int c = 1; c < linesplit.size() && c < 9; c++) {
                            _msolar(row, c - 1) = atof(linesplit[c].c_str());
                        }
                        break;
                    case 2: //hdbt = [12 x 24] mean monthly dry bulb temp for each of the 24 hours of the day (C)
                        for (unsigned int c = 1; c < linesplit.size() && c < 25; c++) {
                            _mhdbt(row, c - 1) = atof(linesplit[c].c_str());
                        }
                        break;
                    case 3: //hEgh =[12 x 24] mean monthly Global Horizontal Radiation for each of the 24 hours of the day (W/m2)
                        for (unsigned int c = 1; c < linesplit.size() && c < 25; c++) {
                            _mhEgh(row, c - 1) = atof(linesplit[c].c_str());
                        }
                        break;
                    case 4:  //megh = [12 x 1] mean monthly Global Horizontal Radiation (W/m2)
                        _mEgh[row] = atof(linesplit[1].c_str());
                        break;
                    case 5:    //mdbt = [12 x 1] mean monthly dry bulb temp (C)
                        _mdbt[row] = atof(linesplit[1].c_str());
                        break;
                    case 6:    //mwind = [12 x 1] mean monthly wind speed; (m/s) 
                        _mwind[row] = atof(linesplit[1].c_str());
                        break;
                    default:
                        break;
                    }
                    row++;
                }
            }
            _weather->setMdbt(_mdbt);
            _weather->setMEgh(_mEgh);
            _weather->setMhdbt(_mhdbt);
            _weather->setMhEgh(_mhEgh);
            _weather->setMsolar(_msolar);
            _weather->setMwind(_mwind);
        }

        void UserModel::load(std::string buildingFile)
        {
            dataFile = buildingFile;
            _valid = true;
            if (!std::filesystem::exists(buildingFile)) {
                std::cout << "ISO Model File Not Found: " << buildingFile << std::endl;
                _valid = false;
                return;
            }
            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << "Loading Building File: " << buildingFile << std::endl;
            loadBuilding(buildingFile);
            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << "Loading Weather File: " << weatherFilePath() << std::endl;
            loadWeather();
            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << "Weather File Loaded" << std::endl;
        }

        void UserModel::load(std::string buildingFile, std::string defaultsFile)
        {
            dataFile = buildingFile;
            _valid = true;

            // Check for the .ism file.
            if (!std::filesystem::exists(buildingFile)) {
                std::cout << "ISO Model File Not Found: " << buildingFile << std::endl;
                _valid = false;
                return;
            }

            // Check for the defaults file.
            if (!std::filesystem::exists(defaultsFile)) {
                std::cout << "ISO Model File Not Found: " << defaultsFile << std::endl;
                _valid = false;
                return;
            }

            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << "Loading Building File: " << buildingFile << std::endl;

            loadBuilding(buildingFile, defaultsFile);

            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << "Loading Weather File: " << weatherFilePath() << std::endl;

            loadWeather();

            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << "Weather File Loaded" << std::endl;
        }
    } // isomodel
} // openstudio