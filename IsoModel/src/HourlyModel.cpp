/*
 * HourlyModel.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: craig
 */

#include "HourlyModel.hpp"

namespace openstudio {
    namespace isomodel {

        //TODO This initializer list should be removed and these attributes included in the ism file. -BAA@2014-12-14
        // There are a bunch more similar constants that are initialized in HourlyModel::initialize().
        HourlyModel::HourlyModel() {}
        HourlyModel::~HourlyModel() {}

        std::vector<EndUses> HourlyModel::simulate(bool aggregateByMonth)
        {
            populateSchedules();

            printMatrix("Cooling Setpoint", (double*)fixedActualCoolingSetpoint, 24, 7);
            printMatrix("Heating Setpoint", (double*)fixedActualHeatingSetpoint, 24, 7);
            printMatrix("Exterior Equipment", (double*)fixedExteriorEquipmentSchedule, 24, 7);
            printMatrix("Exterior Lighting", (double*)fixedExteriorLightingSchedule, 24, 7);
            printMatrix("Interior Equipment", (double*)fixedInteriorEquipmentSchedule, 24, 7);
            printMatrix("Interior Lighting", (double*)fixedInteriorLightingSchedule, 24, 7);
            printMatrix("Ventilation", (double*)fixedVentilationSchedule, 24, 7);

            initialize();
            TimeFrame frame;
            double TMT1 = 20.0;
            double tiHeatCool = 20.0;

            // Avoid copying EPW data vectors
            const auto& data = epwData->data();
            const std::vector<double>& wind = data[WSPD];
            const std::vector<double>& temp = data[DBT];
            const std::vector<double>& egh = data[EGH]; // global horizontal

            SolarRadiation pos(&frame, epwData.get());
            pos.Calculate();
            auto eglobe = pos.eglobe(); // Radiation for 8 directions (N, NE, E, etc.).

            HourResults<double> tempHourResults;
            HourResults<std::vector<double>> rawResults;

            // Pre-reserve space for hourly results
            rawResults.Qneed_ht.reserve(TIMESLICES);
            rawResults.Qneed_cl.reserve(TIMESLICES);
            rawResults.Q_illum_tot.reserve(TIMESLICES);
            rawResults.Q_illum_ext_tot.reserve(TIMESLICES);
            rawResults.Qfan_tot.reserve(TIMESLICES);
            rawResults.Qpump_tot.reserve(TIMESLICES);
            rawResults.phi_plug.reserve(TIMESLICES);
            rawResults.externalEquipmentEnergyWperm2.reserve(TIMESLICES);
            rawResults.Q_dhw.reserve(TIMESLICES);

            std::array<double, 9> solarRadiationForHour;

            const double floorArea = structure.floorArea(); // reused in calculateHour via captured reference (if desired)

            for (int i = 0; i < TIMESLICES; ++i) {
                // Prepare 9-direction radiation vector per hour without allocations
                solarRadiationForHour[0] = eglobe[i][0];
                solarRadiationForHour[1] = eglobe[i][1];
                solarRadiationForHour[2] = eglobe[i][2];
                solarRadiationForHour[3] = eglobe[i][3];
                solarRadiationForHour[4] = eglobe[i][4];
                solarRadiationForHour[5] = eglobe[i][5];
                solarRadiationForHour[6] = eglobe[i][6];
                solarRadiationForHour[7] = eglobe[i][7];
                solarRadiationForHour[8] = egh[i]; // roof

                int month = frame.Month[i];
                int hourOfDay = frame.Hour[i];
                int dayOfWeek = frame.DayOfWeek[i];

                calculateHour(i + 1,          // hourOfYear
                    month,          // month
                    dayOfWeek,      // dayOfWeek
                    hourOfDay,      // hourOfDay
                    wind[i],        // windMps
                    temp[i],        // temperature
                    solarRadiationForHour,
                    TMT1,
                    tiHeatCool,
                    tempHourResults);

                // Store each result type in its own vector.
                rawResults.Qneed_ht.push_back(tempHourResults.Qneed_ht);
                rawResults.Qneed_cl.push_back(tempHourResults.Qneed_cl);
                rawResults.Q_illum_tot.push_back(tempHourResults.Q_illum_tot);
                rawResults.Q_illum_ext_tot.push_back(tempHourResults.Q_illum_ext_tot);
                rawResults.Qfan_tot.push_back(tempHourResults.Qfan_tot);
                rawResults.Qpump_tot.push_back(tempHourResults.Qpump_tot);
                rawResults.phi_plug.push_back(tempHourResults.phi_plug);
                rawResults.externalEquipmentEnergyWperm2.push_back(tempHourResults.externalEquipmentEnergyWperm2);
                rawResults.Q_dhw.push_back(tempHourResults.Q_dhw);
            }

            // Factor the raw need results by the distribution efficiencies.
            auto a_ht_loss = heating.hvacLossFactor();
            auto a_cl_loss = cooling.hvacLossFactor();
            auto f_waste = heating.hotcoldWasteFactor();
            auto cop = cooling.cop();
            auto efficiency_ht = heating.efficiency();

            // Calculate the yearly totals.
            auto Qneed_ht_yr = std::accumulate(rawResults.Qneed_ht.begin(), rawResults.Qneed_ht.end(), 0.0);
            auto Qneed_cl_yr = std::accumulate(rawResults.Qneed_cl.begin(), rawResults.Qneed_cl.end(), 0.0);

            auto f_dem_ht = std::max(Qneed_ht_yr / (Qneed_cl_yr + Qneed_ht_yr), 0.1);
            auto f_dem_cl = std::max((1.0 - f_dem_ht), 0.1);

            auto eta_dist_ht = 1.0 / (1.0 + a_ht_loss + f_waste / f_dem_ht);
            auto eta_dist_cl = 1.0 / (1.0 + a_cl_loss + f_waste / f_dem_cl);

            // Create unary functions to factor the heating and cooling values.
            auto factorHeating = [=](double need) { return need / eta_dist_ht / efficiency_ht; };
            auto factorCooling = [=](double need) { return need / eta_dist_cl / cop; };

            // Create containers for factored heating and cooling values.
            std::vector<double> v_Qht_sys(rawResults.Qneed_ht.size());
            std::vector<double> v_Qcl_sys(rawResults.Qneed_cl.size());

            // Factor the heating and cooling values.
            std::transform(rawResults.Qneed_ht.begin(), rawResults.Qneed_ht.end(), v_Qht_sys.begin(), factorHeating);
            std::transform(rawResults.Qneed_cl.begin(), rawResults.Qneed_cl.end(), v_Qcl_sys.begin(), factorCooling);

            // Store the factored results and rename them to match the monthly result names.
            std::map<std::string, std::vector<double>> results;

            // TODO Fix this! Hardcoded values of '0' for things not being calculated is not ideal.
            std::vector<double> zeroes(rawResults.Qneed_ht.size(), 0.0);

            results["Eelec_ht"] = (heating.energyType() == 1) ? v_Qht_sys : zeroes; // If electric.
            results["Eelec_cl"] = v_Qcl_sys;
            results["Eelec_int_lt"] = rawResults.Q_illum_tot;
            results["Eelec_ext_lt"] = rawResults.Q_illum_ext_tot;
            results["Eelec_fan"] = rawResults.Qfan_tot;
            results["Eelec_pump"] = rawResults.Qpump_tot;
            results["Eelec_int_plug"] = rawResults.phi_plug;
            results["Eelec_ext_plug"] = rawResults.externalEquipmentEnergyWperm2; // Currently hardcoded.
            results["Eelec_dhw"] = rawResults.Q_dhw;
            results["Egas_ht"] = (heating.energyType() != 1) ? v_Qht_sys : zeroes; // If not electric.
            results["Egas_cl"] = zeroes;
            results["Egas_plug"] = zeroes;
            results["Egas_dhw"] = zeroes;

            // Convert to EUI in kWh/m^2
            constexpr double W_TO_KWH = 1.0 / 1000.0;
            for (auto& kv : results) {
                auto& vec = kv.second;
                for (double& v : vec) {
                    v *= W_TO_KWH;
                }
            }

            if (aggregateByMonth) {
                // Calculate monthly results
                for (const auto& kv : results) {
                    results[kv.first] = sumHoursByMonth(kv.second);
                }
            }

            int numberOfResults = aggregateByMonth ? 12 : 8760;

            // Cache references to avoid repeated map lookups in tight loop
            auto& v_Eelec_ht = results["Eelec_ht"];
            auto& v_Eelec_cl = results["Eelec_cl"];
            auto& v_Eelec_int_lt = results["Eelec_int_lt"];
            auto& v_Eelec_ext_lt = results["Eelec_ext_lt"];
            auto& v_Eelec_fan = results["Eelec_fan"];
            auto& v_Eelec_pump = results["Eelec_pump"];
            auto& v_Eelec_int_plug = results["Eelec_int_plug"];
            auto& v_Eelec_ext_plug = results["Eelec_ext_plug"];
            auto& v_Eelec_dhw = results["Eelec_dhw"];
            auto& v_Egas_ht = results["Egas_ht"];
            auto& v_Egas_cl = results["Egas_cl"];
            auto& v_Egas_plug = results["Egas_plug"];
            auto& v_Egas_dhw = results["Egas_dhw"];

            std::vector<EndUses> allResults;
            allResults.reserve(numberOfResults);

            for (int i = 0; i < numberOfResults; ++i) {
                EndUses timestepEndUses;
#ifdef ISOMODEL_STANDALONE
                auto euse = 0;
                timestepEndUses.addEndUse(euse++, v_Eelec_ht[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_cl[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_int_lt[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_ext_lt[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_fan[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_pump[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_int_plug[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_ext_plug[i]);
                timestepEndUses.addEndUse(euse++, v_Eelec_dhw[i]);
                timestepEndUses.addEndUse(euse++, v_Egas_ht[i]);
                timestepEndUses.addEndUse(euse++, v_Egas_cl[i]);
                timestepEndUses.addEndUse(euse++, v_Egas_plug[i]);
                timestepEndUses.addEndUse(euse++, v_Egas_dhw[i]);
#else
                timestepEndUses.addEndUse(v_Eelec_ht[i], EndUseFuelType::Electricity, EndUseCategoryType::Heating);
                timestepEndUses.addEndUse(v_Eelec_cl[i], EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
                timestepEndUses.addEndUse(v_Eelec_int_lt[i], EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
                timestepEndUses.addEndUse(v_Eelec_ext_lt[i], EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
                timestepEndUses.addEndUse(v_Eelec_fan[i], EndUseFuelType::Electricity, EndUseCategoryType::Fans);
                timestepEndUses.addEndUse(v_Eelec_pump[i], EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
                timestepEndUses.addEndUse(v_Eelec_int_plug[i], EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
                timestepEndUses.addEndUse(v_Eelec_ext_plug[i], EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
                timestepEndUses.addEndUse(v_Eelec_dhw[i], EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);

                timestepEndUses.addEndUse(v_Egas_ht[i], EndUseFuelType::Gas, EndUseCategoryType::Heating);
                timestepEndUses.addEndUse(v_Egas_cl[i], EndUseFuelType::Gas, EndUseCategoryType::Cooling);
                timestepEndUses.addEndUse(v_Egas_plug[i], EndUseFuelType::Gas, EndUseCategoryType::InteriorEquipment);
                timestepEndUses.addEndUse(v_Egas_dhw[i], EndUseFuelType::Gas, EndUseCategoryType::WaterSystems);
#endif
                allResults.push_back(timestepEndUses);
            }
            return allResults;
        }

        void HourlyModel::calculateHour(int hourOfYear,
            int month,
            int dayOfWeek,
            int hourOfDay,
            double windMps,
            double temperature,
            const std::array<double, 9>& solarRadiation,
            double& TMT1,
            double& tiHeatCool,
            HourResults<double>& results)
        {
            // scheduleOffset appears to perhaps be supposed to convert a 0 to 6, Sunday to Saturday range into a 1 to 7, Monday to Sunday 
            // range, but because dayOfWeek is a 1-7 range, it does nothing. BAA@2015-04-15.
            auto scheduleOffset = dayOfWeek;

            const double floorArea = structure.floorArea();

            // Extract schedules
            // Convert ventilation from L/s to m^3/h and divide by floor area.
            auto ventExhaustM3phpm2 = ventilationSchedule(hourOfYear, hourOfDay, scheduleOffset) * 3.6 / floorArea;
            auto externalEquipmentPower = exteriorEquipmentSchedule(hourOfYear, hourOfDay, scheduleOffset);
            auto interiorEquipmentPowerDensity = interiorEquipmentSchedule(hourOfYear, hourOfDay, scheduleOffset);
            auto exteriorLightingEnabled = exteriorLightingSchedule(hourOfYear, hourOfDay, scheduleOffset);
            auto interiorLightingPowerDensity = interiorLightingSchedule(hourOfYear, hourOfDay, scheduleOffset);
            auto actualHeatingSetpoint = heatingSetpointSchedule(hourOfYear, hourOfDay, scheduleOffset);
            auto actualCoolingSetpoint = coolingSetpointSchedule(hourOfYear, hourOfDay, scheduleOffset);

            results.externalEquipmentEnergyWperm2 = externalEquipmentPower / floorArea;

            // \Phi_{int,A}, ISO 13790 10.4.2.
            results.phi_plug = interiorEquipmentPowerDensity;

            const double invAreaRatio = 53.0 / areaNaturallyLightedRatio;
            const double maxIrrad = structure.irradianceForMaxShadingUse();

            // Compute lightingLevel and solar heat gain without intermediate vectors
            double lightingLevel = 0.0;
            double qSolarHeatGain = 0.0;

            for (int i = 0; i != 9; ++i) {
                const double sr = solarRadiation[i];
                const double srCl = std::min(maxIrrad, sr);

                // Lighting
                double nlTerm = naturalLightRatio[i] +
                    shadingUsePerWPerM2 * naturalLightShadeRatioReduction[i] * srCl;
                lightingLevel += invAreaRatio * sr * nlTerm;

                // Solar heat gain
                double sTerm = solarRatio[i] +
                    solarShadeRatioReduction[i] * shadingUsePerWPerM2 * srCl;
                qSolarHeatGain += sr * sTerm;
            }

            auto electricForNaturalLightArea = std::max(0.0, maxRatioElectricLighting * (1 - lightingLevel / elightNatural));
            auto electricForTotalLightArea = electricForNaturalLightArea * areaNaturallyLightedRatio
                + (1 - areaNaturallyLightedRatio) * maxRatioElectricLighting;

            // Heat produced by lighting.
            auto phi_illum = electricForTotalLightArea * interiorLightingPowerDensity * lights.elecInternalGains();

            results.Q_illum_tot = electricForTotalLightArea * interiorLightingPowerDensity;

            // \Phi_{int}
            auto phi_int = results.phi_plug + phi_illum;

            // \Phi_{ia}
            auto phii = simSettings.phiSolFractionToAirNode() * qSolarHeatGain +
                simSettings.phiIntFractionToAirNode() * phi_int;
            auto phii10 = phii + 10;

            // Ventilation from wind. ISO 15242.
            auto qSupplyBySystem = ventExhaustM3phpm2 * windImpactSupplyRatio;
            auto exhaustSupply = -(qSupplyBySystem - ventExhaustM3phpm2); // ISO 15242 q_{v-diff}.
            auto tAfterExchange = (1 - ventilation.heatRecoveryEfficiency()) * temperature +
                ventilation.heatRecoveryEfficiency() * 20;
            auto tSuppliedAir = std::max(ventilation.ventPreheatDegC(), tAfterExchange);

            // ISO 15242 6.7.1 Step 1.
            auto qWind = 0.0769 * q4Pa * std::pow((ventilation.dCp() * windMps * windMps), 0.667);
            auto qStackPrevIntTemp = 0.0146 * q4Pa *
                std::pow((0.5 * windImpactHz * (std::max(0.00001, std::fabs(temperature - tiHeatCool)))), 0.667);

            // ISO 15242 6.7.1 Step 2.
            auto qExfiltration = std::max(0.0,
                std::max(qStackPrevIntTemp, qWind) - std::fabs(exhaustSupply) *
                (0.5 * qStackPrevIntTemp + 0.667 * qWind / (qStackPrevIntTemp + qWind)));

            auto qEnvelope = std::max(0.0, exhaustSupply) + qExfiltration;
            auto qEnteringTotal = qEnvelope + qSupplyBySystem;

            // \theta_{sup} ISO 13790 9.3.
            auto tEnteringAndSupplied = (temperature * qEnvelope + tSuppliedAir * qSupplyBySystem) / qEnteringTotal;
            auto hei = 0.34 * qEnteringTotal;

            auto h1 = 1 / (1 / hei + 1 / H_tris);          // H_{tr,1}
            auto h2 = h1 + hwindowWperkm2;                 // H_{tr,2}

            // \Phi_{st}, \Phi_{m}
            auto phisPhi0 = prsSolar * qSolarHeatGain + prsInterior * phi_int;
            auto phimPhi0 = prmSolar * qSolarHeatGain + prmInterior * phi_int;

            auto h3 = 1 / (1 / h2 + 1 / H_ms);             // H_{tr,3}

            auto phimTotalPhi10 = phimPhi0 + hem * temperature
                + h3 * (phisPhi0 + hwindowWperkm2 * temperature +
                    h1 * (phii10 / hei + tEnteringAndSupplied)) / h2;
            auto phimTotalPhi0 = phimPhi0 + hem * temperature
                + h3 * (phisPhi0 + hwindowWperkm2 * temperature +
                    h1 * (phii / hei + tEnteringAndSupplied)) / h2;

            auto CmTerm = Cm / 3.6;
            auto h3hemHalf = 0.5 * (h3 + hem);

            // \theta_{m,t10}, \theta_{m,t}
            auto tmt1Phi10 = (TMT1 * (CmTerm - h3hemHalf) + phimTotalPhi10) / (CmTerm + h3hemHalf);
            auto tmPhi10 = 0.5 * (TMT1 + tmt1Phi10);

            auto tsPhi10 = (H_ms * tmPhi10 + phisPhi0 +
                hwindowWperkm2 * temperature +
                h1 * (tEnteringAndSupplied + phii10 / hei)) /
                (H_ms + hwindowWperkm2 + h1);
            auto tiPhi10 = (H_tris * tsPhi10 + hei * tEnteringAndSupplied + phii10) /
                (H_tris + hei);

            auto tmt1Phi0 = (TMT1 * (CmTerm - h3hemHalf) + phimTotalPhi0) / (CmTerm + h3hemHalf);
            auto tmPhi0 = 0.5 * (TMT1 + tmt1Phi0);

            auto tsPhi0 = (H_ms * tmPhi0 + phisPhi0 +
                hwindowWperkm2 * temperature +
                h1 * (tEnteringAndSupplied + phii / hei)) /
                (H_ms + hwindowWperkm2 + h1);
            auto tiPhi0 = (H_tris * tsPhi0 + hei * tEnteringAndSupplied + phii) /
                (H_tris + hei);

            auto denom = (tiPhi10 - tiPhi0);
            auto phiCooling = 10 * (actualCoolingSetpoint - tiPhi0) / denom;
            auto phiHeating = 10 * (actualHeatingSetpoint - tiPhi0) / denom;
            auto phiActual = std::max(0.0, phiHeating) + std::min(phiCooling, 0.0);

            results.Qneed_cl = std::max(0.0, -phiActual);
            results.Qneed_ht = std::max(0.0, phiActual);

            // Fan power
            auto T_sup_ht = heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
            auto T_sup_cl = cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

            double rhoCpAir_277 = phys.rhoCpAir() * 277.777778;

            auto Vair_ht = heating.forcedAirHeating() ?
                results.Qneed_ht / (((T_sup_ht - tiHeatCool) * rhoCpAir_277) + DBL_MIN) :
                0.0;
            auto Vair_cl = cooling.forcedAirCooling() ?
                results.Qneed_cl / (((tiHeatCool - T_sup_cl) * rhoCpAir_277) + DBL_MIN) :
                0.0;

            auto Vair_tot = std::max(Vair_ht + Vair_cl, ventExhaustM3phpm2);

            // fan energy in W/m2
            results.Qfan_tot = Vair_tot * ventilation.fanPower() * 1000.0 / 3600.0;

            // Pump energy
            if (results.Qneed_cl > 0.0) {
                results.Qpump_tot = cooling.E_pumps() * cooling.pumpControlReduction();
            }
            else if (results.Qneed_ht > 0.0) {
                results.Qpump_tot = heating.E_pumps() * heating.pumpControlReduction();
            }
            else {
                results.Qpump_tot = 0.0;
            }

            if (solarRadiation[8] > 0) { // roof radiation: sun is up
                results.Q_illum_ext_tot = 0; // No exterior lights during the day.
            }
            else {
                results.Q_illum_ext_tot = lights.exteriorEnergy() * exteriorLightingEnabled / floorArea;
            }

            results.Q_dhw = 0; //TODO no DHW calculations

            // Update tiHeatCool & TMT1 for next hour.
            auto phiiHeatCool = phiActual + phii;

            auto phimHeatCoolTotal = phimPhi0 + hem * temperature
                + h3 * (phisPhi0 + hwindowWperkm2 * temperature +
                    h1 * (phiiHeatCool / hei + tEnteringAndSupplied)) / h2;

            auto tmt = TMT1;
            TMT1 = (TMT1 * (CmTerm - h3hemHalf) + phimHeatCoolTotal) / (CmTerm + h3hemHalf);

            auto tmHeatCool = 0.5 * (TMT1 + tmt);
            auto tsHeatCool = (H_ms * tmHeatCool + phisPhi0 +
                hwindowWperkm2 * temperature +
                h1 * (tEnteringAndSupplied + phiiHeatCool / hei)) /
                (H_ms + hwindowWperkm2 + h1);
            tiHeatCool = (H_tris * tsHeatCool + hei * tEnteringAndSupplied + phiiHeatCool) /
                (H_tris + hei);
        }


        void HourlyModel::initialize()
        {
            auto lightingOccupancySensorDimmingFraction = building.lightingOccupancySensor();
            auto daylightSensorDimmingFraction = lights.dimmingFraction();

            if (lightingOccupancySensorDimmingFraction < 1.0 &&
                daylightSensorDimmingFraction < 1.0) {
                maxRatioElectricLighting = lights.presenceAutoAd();
                elightNatural = lights.presenceAutoLux();
            }
            else if (lightingOccupancySensorDimmingFraction < 1.0) {
                maxRatioElectricLighting = lights.presenceSensorAd();
                elightNatural = lights.presenceSensorLux();
            }
            else if (daylightSensorDimmingFraction < 1.0) {
                maxRatioElectricLighting = lights.automaticAd();
                elightNatural = lights.automaticLux();
            }
            else {
                maxRatioElectricLighting = lights.manualSwitchAd();
                elightNatural = lights.manualSwitchLux();
            }

            const double floorArea = structure.floorArea();

            areaNaturallyLighted = std::max(0.0001, lights.naturallyLightedArea());
            areaNaturallyLightedRatio = areaNaturallyLighted / floorArea;

            for (int i = 0; i != 9; ++i) {
                structureCalculations(structure.windowShadingDevice()[i],
                    structure.wallArea()[i],
                    structure.windowArea()[i],
                    structure.wallUniform()[i],
                    structure.windowUniform()[i],
                    structure.wallSolarAbsorption()[i],
                    structure.windowShadingCorrectionFactor()[i],
                    structure.windowNormalIncidenceSolarEnergyTransmittance()[i],
                    i);

                nlaWMovableShading[i] = nlams[i] / floorArea;
                naturalLightRatio[i] = nla[i] / floorArea;
                naturalLightShadeRatioReduction[i] = nlaWMovableShading[i] - naturalLightRatio[i];

                saWMovableShading[i] = sams[i] / floorArea;
                solarRatio[i] = sa[i] / floorArea;
                solarShadeRatioReduction[i] = saWMovableShading[i] - solarRatio[i];
            }

            shadingUsePerWPerM2 = structure.shadingFactorAtMaxUse() /
                structure.irradianceForMaxShadingUse();

            // ISO 15242 Air leakage values.
            auto buildingv8 = 0.19 * (ventilation.n50() * (floorArea * structure.buildingHeight()));
            q4Pa = std::max(0.000001, buildingv8 / floorArea);

            // ISO 13790 12.2.2
            h_ms = simSettings.hci() + simSettings.hri() * 1.2;
            h_is = 1 / (1 / simSettings.hci() - 1 / h_ms);
            H_tris = h_is * structure.totalAreaPerFloorArea();

            // Calculate Cm
            auto Cm_int = structure.interiorHeatCapacity() / 1000.0;
            auto Cm_env = (structure.wallHeatCapacity() * sum(structure.wallArea()) / floorArea) / 1000.0;
            Cm = Cm_int + Cm_env;

            if (Cm > 370.0) {
                Am = 3.5;
            }
            else if (Cm > 260.0) {
                Am = 3.0 + 0.5 * ((Cm - 260) / 110);
            }
            else if (Cm > 165.0) {
                Am = 2.5 + 0.5 * ((Cm - 165) / 95);
            }
            else {
                Am = 2.5;
            }

            double hWind = 0.0;
            double hWall = 0.0;

            for (int i = 0; i != 9; ++i) {
                hWind += hWindow[i];
                hWall += htot[i] - hWindow[i];
            }
            hwindowWperkm2 = hWind / floorArea;

            prs = (structure.totalAreaPerFloorArea() - Am - hwindowWperkm2 / h_ms) /
                structure.totalAreaPerFloorArea();
            prsInterior = (1 - simSettings.phiIntFractionToAirNode()) * prs;
            prsSolar = (1 - simSettings.phiSolFractionToAirNode()) * prs;

            prm = Am / structure.totalAreaPerFloorArea();
            prmInterior = (1 - simSettings.phiIntFractionToAirNode()) * prm;
            prmSolar = (1 - simSettings.phiSolFractionToAirNode()) * prm;

            H_ms = h_ms * Am;

            hOpaqueWperkm2 = std::max(hWall / floorArea, 0.000001);

            hem = 1 / (1 / hOpaqueWperkm2 - 1 / H_ms);

            windImpactHz = std::max(0.1, ventilation.hzone());
            windImpactSupplyRatio = std::max(0.00001, ventilation.fanControlFactor());
        }

        void HourlyModel::populateSchedules()
        {
            const int dayStart = static_cast<int>(pop.daysStart());
            const int dayEnd = static_cast<int>(pop.daysEnd());
            const int hourStart = static_cast<int>(pop.hoursStart());
            const int hourEnd = static_cast<int>(pop.hoursEnd());

            const double ventRate = ventilation.supplyRate();
            const double extEquip = building.externalEquipment();
            const double intOcc = building.electricApplianceHeatGainOccupied();
            const double intUnocc = building.electricApplianceHeatGainUnoccupied();
            const double intLtOcc = lights.powerDensityOccupied();
            const double intLtUnocc = lights.powerDensityUnoccupied();
            const double htOcc = heating.temperatureSetPointOccupied();
            const double htUnocc = heating.temperatureSetPointUnoccupied();
            const double clOcc = cooling.temperatureSetPointOccupied();
            const double clUnocc = cooling.temperatureSetPointUnoccupied();

            for (int h = 0; h < 24; ++h) {
                bool hoccupied = (h >= hourStart && h <= hourEnd);
                for (int d = 0; d < 7; ++d) {
                    bool doccupied = (d >= dayStart && d <= dayEnd);
                    bool popoccupied = hoccupied && doccupied;

                    fixedVentilationSchedule[h][d] = hoccupied ? ventRate : 0.0;
                    fixedExteriorEquipmentSchedule[h][d] = extEquip;
                    fixedInteriorEquipmentSchedule[h][d] = popoccupied ? intOcc : intUnocc;
                    fixedExteriorLightingSchedule[h][d] = 1; // lights only actually used at night in calculateHour
                    fixedInteriorLightingSchedule[h][d] = popoccupied ? intLtOcc : intLtUnocc;
                    fixedActualHeatingSetpoint[h][d] = popoccupied ? htOcc : htUnocc;
                    fixedActualCoolingSetpoint[h][d] = popoccupied ? clOcc : clUnocc;
                }
            }
        }

        void HourlyModel::structureCalculations(double SHGC,
            double wallAreaM2,
            double windowAreaM2,
            double wallUValue,
            double windowUValue,
            double wallSolarAbsorption,
            double solarFactorWith,
            double solarFactorWithout,
            int direction)
        {
            double WindowT = SHGC / 0.87;
            nlams[direction] = windowAreaM2 * WindowT; // Natural lighted area movable shade.
            nla[direction] = windowAreaM2 * WindowT; // Natural lighted area.
            sams[direction] = wallAreaM2 * (wallSolarAbsorption * wallUValue * structure.R_se()) + windowAreaM2 * solarFactorWith;
            sa[direction] = wallAreaM2 * (wallSolarAbsorption * wallUValue * structure.R_se()) + windowAreaM2 * solarFactorWithout;
            htot[direction] = wallAreaM2 * wallUValue + windowAreaM2 * windowUValue;
            hWindow[direction] = windowAreaM2 * windowUValue;
        }

//        // TODO BAA@2015-01-28 Is there a better place to keep these debug functions?
//        void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2)
//        {
//#ifdef DEBUG_ISO_MODEL_SIMULATION
//            std::cout << matName << "(" << dim1 << ", " << dim2 << "): " << std::endl << "\t";
//            for (unsigned int j = 0; j < dim2; j++) {
//                std::cout << "," << j;
//            }
//            std::cout << std::endl;
//            for (unsigned int i = 0; i < dim1; ++i) {
//                std::cout << "\t" << i;
//                for (unsigned int j = 0; j < dim2; j++) {
//                    std::cout << "," << mat[i * dim2 + j];
//                }
//                std::cout << std::endl;
//            }
//#else
//            (void)matName; (void)mat; (void)dim1; (void)dim2;
//#endif
//        }
// 
//        // TODO BAA@2015-01-28 Is there a better place to keep these debug functions?
        void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2)
        {
        }

        std::vector<double> HourlyModel::sumHoursByMonth(const std::vector<double>& hourlyData)
        {
            std::vector<double> monthlyData(12);
            static const int monthsInHours[13] = { 0, 744, 1416, 2160, 2880, 3624, 4344,
                                                   5088, 5832, 6552, 7296, 8016, 8760 };

            for (int month = 0; month < 12; ++month) {
                monthlyData[month] =
                    std::accumulate(hourlyData.begin() + monthsInHours[month],
                        hourlyData.begin() + monthsInHours[month + 1],
                        0.0);
            }

            return monthlyData;
        }

    }
}