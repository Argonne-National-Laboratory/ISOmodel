/*
 * HourlyModel.cpp
 *
 * Optimized Refactor
 * Changes:
 * 1. Vectorized Solar Loop: Removed 'if' inside direction loop; used std::min for shading to allow SIMD.
 * 2. Branchless Arithmetic: Added epsilon (1e-15) to denominators to eliminate 'if' checks and pipeline stalls.
 * 3. Schedule Flattening: Flattened [24][7] lookups into 8760 linear arrays for max L1 cache hits.
 * 4. Math Hoisting: Pre-calculated static thermal components and loop-invariant logic.
 */

#include "HourlyModel.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat> // DBL_MIN
#include <vector>

namespace openstudio {
    namespace isomodel {

        // Stub for debug printing
        void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2) {}

        HourlyModel::HourlyModel() : invFloorArea(0), rhoCpAir_277(0) {}
        HourlyModel::~HourlyModel() {}

        std::vector<EndUses> HourlyModel::simulate(bool aggregateByMonth)
        {
            populateSchedules();
            initialize(); // Precomputes constants (invFloorArea, Cm, etc.)

            TimeFrame frame;
            double TMT1 = 20.0;
            double tiHeatCool = 20.0;

            // Direct access to EPW data
            const auto& data = epwData->data();
            const std::vector<double>& wind = data[WSPD];
            const std::vector<double>& temp = data[DBT];
            const std::vector<double>& egh = data[EGH];

            SolarRadiation pos(&frame, epwData.get());
            pos.Calculate();
            const auto& eglobe = pos.eglobe();

            // --- CHANGE 3: Flatten Schedules for Cache Locality ---
            // Moving 2D [24][7] lookup to a linear [8760] sweep to prevent repeated index calculations
            std::vector<double> flatVent(8760), flatExtEq(8760), flatIntEq(8760),
                flatExtLt(8760), flatIntLt(8760), flatHeatSP(8760), flatCoolSP(8760);

            for (int i = 0; i < 8760; ++i) {
                int h = frame.Hour[i];
                int d = frame.DayOfWeek[i];
                flatVent[i] = fixedVentilationSchedule[h][d];
                flatExtEq[i] = fixedExteriorEquipmentSchedule[h][d];
                flatIntEq[i] = fixedInteriorEquipmentSchedule[h][d];
                flatExtLt[i] = fixedExteriorLightingSchedule[h][d];
                flatIntLt[i] = fixedInteriorLightingSchedule[h][d];
                flatHeatSP[i] = fixedActualHeatingSetpoint[h][d];
                flatCoolSP[i] = fixedActualCoolingSetpoint[h][d];
            }

            // 1. PREALLOCATE RESULTS
            size_t numHours = 8760;
            std::vector<double> r_Qneed_ht(numHours);
            std::vector<double> r_Qneed_cl(numHours);
            std::vector<double> r_Q_illum_tot(numHours);
            std::vector<double> r_Q_illum_ext_tot(numHours);
            // ... other vectors remain as SoA for performance
            std::vector<double> r_Qfan_tot(numHours);
            std::vector<double> r_Qpump_tot(numHours);
            std::vector<double> r_phi_plug(numHours);
            std::vector<double> r_ext_equip(numHours);
            std::vector<double> r_Q_dhw(numHours);

            // Local constants hoisted from loop
            const double iFA = invFloorArea;
            const double maxIrrad = structure.irradianceForMaxShadingUse();
            const double invAreaNat = (areaNaturallyLightedRatio > 0) ? (53.0 / areaNaturallyLightedRatio) : 0.0;
            const double elightNaturalSafe = elightNatural + DBL_MIN;
            const double ventFanFactor = ventilation.fanPower() * 0.277778;

            const double ventHeatRecEff = ventilation.heatRecoveryEfficiency();
            const double ventPreheat = ventilation.ventPreheatDegC();
            const double dCpVent = ventilation.dCp();
            const double pumpControlCl = cooling.pumpControlReduction();
            const double pumpControlHt = heating.pumpControlReduction();
            const double ePumpsCl = cooling.E_pumps();
            const double ePumpsHt = heating.E_pumps();
            const double extLightEnergy = lights.exteriorEnergy();
            const double elecInternalGains = lights.elecInternalGains();

            const double Cm_36 = Cm / 3.6;
            const double T_sup_ht_base = heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
            const double T_sup_cl_base = cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

            // -------------------------------------------------------------------------
            // MAIN SIMULATION LOOP (Inlined calculateHour)
            // -------------------------------------------------------------------------
            for (int i = 0; i < numHours; ++i) {
                double windMps = wind[i];
                double temperature = temp[i];

                // Linear Access (Change 3)
                double schedVent = flatVent[i];
                double schedExtEquip = flatExtEq[i];
                double schedIntEquip = flatIntEq[i];
                double schedIntLight = flatIntLt[i];
                double schedHeatSP = flatHeatSP[i];
                double schedCoolSP = flatCoolSP[i];

                // 1. Equipment & Plug
                r_ext_equip[i] = schedExtEquip * iFA;
                r_phi_plug[i] = schedIntEquip;

                // 2. Lighting & Solar (CHANGE 1: Vectorized Solar)
                // 
                double lightingLevel = 0.0;
                double qSolarHeatGain = 0.0;
                const std::vector<double>& curSolar = eglobe[i];

                for (int k = 0; k < 9; ++k) {
                    double sr = (k < 8) ? curSolar[k] : egh[i];
                    // CHANGE 2: Branchless min to avoid pipeline stalls/mispredictions
                    double srCl = std::min(sr, maxIrrad);
                    lightingLevel += invAreaNat * sr * (naturalLightRatio[k] + shadingUsePerWPerM2 * naturalLightShadeRatioReduction[k] * srCl);
                    qSolarHeatGain += sr * (solarRatio[k] + solarShadeRatioReduction[k] * shadingUsePerWPerM2 * srCl);
                }

                double electricForNaturalLightArea = std::max(0.0, maxRatioElectricLighting * (1.0 - lightingLevel / elightNaturalSafe));
                r_Q_illum_tot[i] = (electricForNaturalLightArea * areaNaturallyLightedRatio + (1.0 - areaNaturallyLightedRatio) * maxRatioElectricLighting) * schedIntLight;
                double phi_int = r_phi_plug[i] + (r_Q_illum_tot[i] * elecInternalGains);

                // Gains to Air Node
                double phii = simSettings.phiSolFractionToAirNode() * qSolarHeatGain + simSettings.phiIntFractionToAirNode() * phi_int;

                // 3. Ventilation & Infiltration
                double ventExh = schedVent * 3.6 * iFA;
                double qSup = ventExh * windImpactSupplyRatio;
                double exhSup = -(qSup - ventExh);
                double tSupp = std::max(ventPreheat, (1.0 - ventHeatRecEff) * temperature + ventHeatRecEff * 20.0);

                // CHANGE 4: Use fastPow0667 approximation
                double qWind = 0.0769 * q4Pa * fastPow23(dCpVent * windMps * windMps);
                double absDT = std::max(std::fabs(temperature - tiHeatCool), 1e-5);
                double qStack = 0.0146 * q4Pa * fastPow23(0.5 * windImpactHz * absDT);

                
                // add (1e-15 to qSW to avoid branch check for divide)
                double qSW = qStack + qWind + 1E-15;
                //double qExf = 0.0;
                // CHANGE 2: Simplified branch for exfiltration to minimize CPU stalls
                //if (qSW > 1e-9) {
                //    qExf = std::max(0.0, std::max(qStack, qWind) - std::fabs(exhSup) * (0.5 * qStack + 0.667 * qWind / qSW));
                //}
                double qExf = std::max(0.0, std::max(qStack, qWind) - std::fabs(exhSup) * (0.5 * qStack + 0.667 * qWind / qSW));

                double qEnt = (exhSup > 0 ? exhSup : 0.0) + qExf + qSup;
                // CHANGE 2: Branchless denominator (1e-15) for tEnt
                double tEnt = (temperature * ((exhSup > 0 ? exhSup : 0.0) + qExf) + tSupp * qSup) / (qEnt + 1e-15);

                // 4. Thermal Network Dynamic Calc
                double hei = 0.34 * qEnt;
                double h1 = 1.0 / (1.0 / (hei + 1e-15) + 1.0 / H_tris);
                double h2 = h1 + hwindowWperkm2;
                double h3 = 1.0 / (1.0 / h2 + 1.0 / H_ms);
                double h3_hem = 0.5 * (h3 + hem);

                double phisPhi0 = prsSolar * qSolarHeatGain + prsInterior * phi_int;
                double phimPhi0 = prmSolar * qSolarHeatGain + prmInterior * phi_int;

                double mid = phisPhi0 + hwindowWperkm2 * temperature + h1 * tEnt;
                double h3_h2 = h3 / h2;
                double t_phi = (h3_h2 * h1) / (hei + 1e-15);

                // Lambda to solve for Ti based on phi input (used for perturbation steps)
                auto solveTi = [&](double pInput) {
                    double phim = phimPhi0 + hem * temperature + h3_h2 * mid + t_phi * pInput;
                    double tmt1 = (TMT1 * (Cm_36 - h3_hem) + phim) / (Cm_36 + h3_hem);
                    double ts = (H_ms * (0.5 * (TMT1 + tmt1)) + mid + h1 * pInput / (hei + 1e-15)) / (H_ms + hwindowWperkm2 + h1);
                    return (H_tris * ts + hei * tEnt + pInput) / (H_tris + hei);
                    };

                double ti_0 = solveTi(phii);
                double ti_10 = solveTi(phii + 10.0);

                // CHANGE 2: Branchless Demand Calculation using epsilon
                double denom = (ti_10 - ti_0) + 1e-15;
                double phiHC = std::max(0.0, 10.0 * (schedHeatSP - ti_0) / denom) + std::min(0.0, 10.0 * (schedCoolSP - ti_0) / denom);

                r_Qneed_ht[i] = std::max(0.0, phiHC);
                r_Qneed_cl[i] = std::max(0.0, -phiHC);

                // Update State
                double phimFinal = phimPhi0 + hem * temperature + h3_h2 * mid + t_phi * (phii + phiHC);
                double tmt1_new = (TMT1 * (Cm_36 - h3_hem) + phimFinal) / (Cm_36 + h3_hem);
                double ts_final = (H_ms * (0.5 * (TMT1 + tmt1_new)) + mid + h1 * (phii + phiHC) / (hei + 1e-15)) / (H_ms + hwindowWperkm2 + h1);

                TMT1 = tmt1_new; // STATE UPDATE
                tiHeatCool = (H_tris * ts_final + hei * tEnt + phii + phiHC) / (H_tris + hei); // STATE UPDATE

                // 5. Fans & Pumps
                double Vair = ventExh;
                Vair = std::max(Vair, r_Qneed_ht[i] / ((T_sup_ht_base - tiHeatCool) * rhoCpAir_277 + DBL_MIN));
                Vair = std::max(Vair, r_Qneed_cl[i] / ((tiHeatCool - T_sup_cl_base) * rhoCpAir_277 + DBL_MIN));
                r_Qfan_tot[i] = Vair * ventFanFactor;

                r_Qpump_tot[i] = (r_Qneed_cl[i] > 0) ? (ePumpsCl * pumpControlCl) : ((r_Qneed_ht[i] > 0) ? (ePumpsHt * pumpControlHt) : 0.0);
                r_Q_illum_ext_tot[i] = (egh[i] > 0) ? 0.0 : (extLightEnergy * flatExtLt[i] * iFA);
            }

            // --- Post Processing ---
            double qh_yr = std::accumulate(r_Qneed_ht.begin(), r_Qneed_ht.end(), 0.0);
            double qc_yr = std::accumulate(r_Qneed_cl.begin(), r_Qneed_cl.end(), 0.0);
            double f_ht = std::max(qh_yr / (qc_yr + qh_yr + DBL_MIN), 0.1);
            double s_ht = (1.0 / (1.0 / (1.0 + heating.hvacLossFactor() + heating.hotcoldWasteFactor() / f_ht))) / heating.efficiency();
            double s_cl = (1.0 / (1.0 / (1.0 + cooling.hvacLossFactor() + heating.hotcoldWasteFactor() / (1.0 - f_ht)))) / cooling.cop();

            bool electricHeat = (heating.energyType() == 1);
            constexpr double W_TO_KWH = 0.001;
            std::vector<EndUses> results;

            if (aggregateByMonth) {
                results.reserve(12);
                const int monthEnds[] = { 0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760 };
                for (int m = 0; m < 12; ++m) {
                    double sum_sys_ht = 0, sum_sys_cl = 0, sum_illum_int = 0, sum_illum_ext = 0, sum_fan = 0, sum_pump = 0, sum_plug_int = 0, sum_plug_ext = 0, sum_dhw = 0;
                    for (int h = monthEnds[m]; h < monthEnds[m + 1]; ++h) {
                        sum_sys_ht += r_Qneed_ht[h]; sum_sys_cl += r_Qneed_cl[h]; sum_illum_int += r_Q_illum_tot[h]; sum_illum_ext += r_Q_illum_ext_tot[h];
                        sum_fan += r_Qfan_tot[h]; sum_pump += r_Qpump_tot[h]; sum_plug_int += r_phi_plug[h]; sum_plug_ext += r_ext_equip[h]; sum_dhw += r_Q_dhw[h];
                    }
                    double elec_ht = electricHeat ? (sum_sys_ht * s_ht * W_TO_KWH) : 0.0;
                    double gas_ht = electricHeat ? 0.0 : (sum_sys_ht * s_ht * W_TO_KWH);
                    EndUses eu;
#ifdef ISOMODEL_STANDALONE
                    eu.addEndUse(0, elec_ht); eu.addEndUse(1, sum_sys_cl * s_cl * W_TO_KWH); eu.addEndUse(2, sum_illum_int * W_TO_KWH); eu.addEndUse(3, sum_illum_ext * W_TO_KWH);
                    eu.addEndUse(4, sum_fan * W_TO_KWH); eu.addEndUse(5, sum_pump * W_TO_KWH); eu.addEndUse(6, sum_plug_int * W_TO_KWH); eu.addEndUse(7, sum_plug_ext * W_TO_KWH);
                    eu.addEndUse(8, sum_dhw * W_TO_KWH); eu.addEndUse(9, gas_ht);
#else
                    eu.addEndUse(elec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
                    eu.addEndUse(sum_sys_cl * s_cl * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
                    eu.addEndUse(sum_illum_int * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
                    eu.addEndUse(sum_illum_ext * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
                    eu.addEndUse(sum_fan * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Fans);
                    eu.addEndUse(sum_pump * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
                    eu.addEndUse(sum_plug_int * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
                    eu.addEndUse(sum_plug_ext * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
                    eu.addEndUse(sum_dhw * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);
                    eu.addEndUse(gas_ht, EndUseFuelType::Gas, EndUseCategoryType::Heating);
#endif
                    results.push_back(eu);
                }
            }
            else {
                results.reserve(8760);
                for (int i = 0; i < 8760; ++i) {
                    EndUses eu;
                    double elec_ht = electricHeat ? (r_Qneed_ht[i] * s_ht * W_TO_KWH) : 0.0;
                    double gas_ht = electricHeat ? 0.0 : (r_Qneed_ht[i] * s_ht * W_TO_KWH);
#ifdef ISOMODEL_STANDALONE
                    eu.addEndUse(0, elec_ht); eu.addEndUse(1, r_Qneed_cl[i] * s_cl * W_TO_KWH); eu.addEndUse(2, r_Q_illum_tot[i] * W_TO_KWH); eu.addEndUse(3, r_Q_illum_ext_tot[i] * W_TO_KWH);
                    eu.addEndUse(4, r_Qfan_tot[i] * W_TO_KWH); eu.addEndUse(5, r_Qpump_tot[i] * W_TO_KWH); eu.addEndUse(6, r_phi_plug[i] * W_TO_KWH); eu.addEndUse(7, r_ext_equip[i] * W_TO_KWH);
                    eu.addEndUse(8, r_Q_dhw[i] * W_TO_KWH); eu.addEndUse(9, gas_ht);
#else
                    eu.addEndUse(elec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
                    eu.addEndUse(r_Qneed_cl[i] * s_cl * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
                    eu.addEndUse(r_Q_illum_tot[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
                    eu.addEndUse(r_Q_illum_ext_tot[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
                    eu.addEndUse(r_Qfan_tot[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Fans);
                    eu.addEndUse(r_Qpump_tot[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
                    eu.addEndUse(r_phi_plug[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
                    eu.addEndUse(r_ext_equip[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
                    eu.addEndUse(r_Q_dhw[i] * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);
                    eu.addEndUse(gas_ht, EndUseFuelType::Gas, EndUseCategoryType::Heating);
#endif
                    results.push_back(eu);
                }
            }
            return results;
        }

        void HourlyModel::initialize() {
            double floorArea = structure.floorArea();
            invFloorArea = (floorArea > 0) ? 1.0 / floorArea : 0.0;
            rhoCpAir_277 = phys.rhoCpAir() * 277.777778;

            auto lightingOccupancySensorDimmingFraction = building.lightingOccupancySensor();
            auto daylightSensorDimmingFraction = lights.dimmingFraction();

            if (lightingOccupancySensorDimmingFraction < 1.0 && daylightSensorDimmingFraction < 1.0) {
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

            areaNaturallyLightedRatio = std::max(0.0001, lights.naturallyLightedArea()) * invFloorArea;

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

                nlaWMovableShading[i] = nlams[i] * invFloorArea;
                naturalLightRatio[i] = nla[i] * invFloorArea;
                naturalLightShadeRatioReduction[i] = nlaWMovableShading[i] - naturalLightRatio[i];

                saWMovableShading[i] = sams[i] * invFloorArea;
                solarRatio[i] = sa[i] * invFloorArea;
                solarShadeRatioReduction[i] = saWMovableShading[i] - solarRatio[i];
            }

            shadingUsePerWPerM2 = structure.shadingFactorAtMaxUse() / structure.irradianceForMaxShadingUse();

            double buildingv8 = 0.19 * (ventilation.n50() * (floorArea * structure.buildingHeight()));
            q4Pa = std::max(0.000001, buildingv8 * invFloorArea);

            h_ms = simSettings.hci() + simSettings.hri() * 1.2;
            h_is = 1.0 / (1.0 / simSettings.hci() - 1.0 / h_ms);
            H_tris = h_is * structure.totalAreaPerFloorArea();

            double Cm_int = structure.interiorHeatCapacity() / 1000.0;
            double Cm_env = (structure.wallHeatCapacity() * sum(structure.wallArea()) * invFloorArea) / 1000.0;
            Cm = Cm_int + Cm_env;

            if (Cm > 370.0) Am = 3.5;
            else if (Cm > 260.0) Am = 3.0 + 0.5 * ((Cm - 260) / 110.0);
            else if (Cm > 165.0) Am = 2.5 + 0.5 * ((Cm - 165) / 95.0);
            else Am = 2.5;

            double hWind = 0.0, hWall = 0.0;
            for (int i = 0; i != 9; ++i) {
                hWind += hWindow[i];
                hWall += htot[i] - hWindow[i];
            }
            hwindowWperkm2 = hWind * invFloorArea;

            prs = (structure.totalAreaPerFloorArea() - Am - hwindowWperkm2 / h_ms) / structure.totalAreaPerFloorArea();
            prsInterior = (1.0 - simSettings.phiIntFractionToAirNode()) * prs;
            prsSolar = (1.0 - simSettings.phiSolFractionToAirNode()) * prs;

            prm = Am / structure.totalAreaPerFloorArea();
            prmInterior = (1.0 - simSettings.phiIntFractionToAirNode()) * prm;
            prmSolar = (1.0 - simSettings.phiSolFractionToAirNode()) * prm;

            H_ms = h_ms * Am;
            hOpaqueWperkm2 = std::max(hWall * invFloorArea, 0.000001);
            hem = 1.0 / (1.0 / hOpaqueWperkm2 - 1.0 / H_ms);

            windImpactHz = std::max(0.1, ventilation.hzone());
            windImpactSupplyRatio = std::max(0.00001, ventilation.fanControlFactor());
        }

        void HourlyModel::populateSchedules() {
            const int dayStart = static_cast<int>(pop.daysStart()), dayEnd = static_cast<int>(pop.daysEnd());
            const int hourStart = static_cast<int>(pop.hoursStart()), hourEnd = static_cast<int>(pop.hoursEnd());
            const double ventRate = ventilation.supplyRate(), extEquip = building.externalEquipment();
            const double intOcc = building.electricApplianceHeatGainOccupied(), intUnocc = building.electricApplianceHeatGainUnoccupied();
            const double intLtOcc = lights.powerDensityOccupied(), intLtUnocc = lights.powerDensityUnoccupied();
            const double htOcc = heating.temperatureSetPointOccupied(), htUnocc = heating.temperatureSetPointUnoccupied();
            const double clOcc = cooling.temperatureSetPointOccupied(), clUnocc = cooling.temperatureSetPointUnoccupied();

            for (int h = 0; h < 24; ++h) {
                bool hoccupied = (h >= hourStart && h <= hourEnd);
                for (int d = 0; d < 7; ++d) {
                    bool doccupied = (d >= dayStart && d <= dayEnd);
                    bool popoccupied = hoccupied && doccupied;
                    fixedVentilationSchedule[h][d] = hoccupied ? ventRate : 0.0;
                    fixedExteriorEquipmentSchedule[h][d] = extEquip;
                    fixedInteriorEquipmentSchedule[h][d] = popoccupied ? intOcc : intUnocc;
                    fixedExteriorLightingSchedule[h][d] = 1;
                    fixedInteriorLightingSchedule[h][d] = popoccupied ? intLtOcc : intLtUnocc;
                    fixedActualHeatingSetpoint[h][d] = popoccupied ? htOcc : htUnocc;
                    fixedActualCoolingSetpoint[h][d] = popoccupied ? clOcc : clUnocc;
                }
            }
        }

        void HourlyModel::structureCalculations(double SHGC, double wallAreaM2, double windowAreaM2,
            double wallUValue, double windowUValue, double wallSolarAbsorption,
            double solarFactorWith, double solarFactorWithout, int direction) {
            double WindowT = SHGC / 0.87;
            nlams[direction] = windowAreaM2 * WindowT;
            nla[direction] = windowAreaM2 * WindowT;
            sams[direction] = wallAreaM2 * (wallSolarAbsorption * wallUValue * structure.R_se()) + windowAreaM2 * solarFactorWith;
            sa[direction] = wallAreaM2 * (wallSolarAbsorption * wallUValue * structure.R_se()) + windowAreaM2 * solarFactorWithout;
            htot[direction] = wallAreaM2 * wallUValue + windowAreaM2 * windowUValue;
            hWindow[direction] = windowAreaM2 * windowUValue;
        }

        std::vector<double> HourlyModel::sumHoursByMonth(const std::vector<double>& hourlyData) { return {}; }

    } // namespace isomodel
} // namespace openstudio