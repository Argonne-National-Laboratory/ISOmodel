/*
 * HourlyModel.cpp
 *
 * Refactored version with modular simulation steps and complete fuel mapping.
 * Updated to use standard C++ <numeric> and <vector> features instead of Boost.
 */

#include "HourlyModel.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat> 
#include <vector>

namespace openstudio {
    namespace isomodel {

        void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2) {}

        HourlyModel::HourlyModel() : invFloorArea(0), rhoCpAir_277(0) {}
        HourlyModel::~HourlyModel() {}

        std::vector<EndUses> HourlyModel::simulate(bool aggregateByMonth)
        {
            populateSchedules();
            initialize();

            TimeFrame frame;
            double TMT1 = 20.0;
            double tiHeatCool = 20.0;

            const auto& data = epwData->data();
            const std::vector<double>& wind = data[WSPD], & temp = data[DBT], & egh = data[EGH];

            SolarRadiation pos(&frame, epwData.get());
            pos.Calculate();
            const auto& eglobe = pos.eglobe();

            size_t numHours = 8760;
            std::vector<double> r_Qneed_ht(numHours), r_Qneed_cl(numHours), r_Q_illum_tot(numHours),
                r_Q_illum_ext_tot(numHours), r_Qfan_tot(numHours), r_Qpump_tot(numHours),
                r_phi_plug(numHours), r_ext_equip(numHours), r_Q_dhw(numHours, 0.0);

            for (int i = 0; i < (int)numHours; ++i) {
                int h = frame.Hour[i], d = frame.DayOfWeek[i];
                double temperature = temp[i];

                r_ext_equip[i] = fixedExteriorEquipmentSchedule[h][d] * invFloorArea;
                r_phi_plug[i] = fixedInteriorEquipmentSchedule[h][d];

                // 1. Gains calculation (Solar and Internal)
                double phii, phi_int, qSolarGain;
                calculateGains(i, eglobe[i], egh[i], fixedInteriorLightingSchedule[h][d], r_phi_plug[i],
                    phi_int, phii, r_Q_illum_tot[i], qSolarGain);

                // 2. Airflow and Ventilation physics
                double ventExh = fixedVentilationSchedule[h][d] * 3.6 * invFloorArea;
                double tEnt, hei, h1;
                calculateAirFlows(wind[i], temperature, tiHeatCool, ventExh, tEnt, hei, h1);

                // 3. Solve RC Network (ISO 13790)
                double phiHC = solveThermalBalance(temperature, tEnt, phii, phi_int, qSolarGain, hei, h1,
                    fixedActualHeatingSetpoint[h][d], fixedActualCoolingSetpoint[h][d],
                    TMT1, tiHeatCool);

                r_Qneed_ht[i] = std::max(0.0, phiHC);
                r_Qneed_cl[i] = std::max(0.0, -phiHC);

                // 4. Auxiliary Systems (Fans and Pumps)
                double Vair = std::max({ ventExh,
                    r_Qneed_ht[i] / (((heating.temperatureSetPointOccupied() + heating.dT_supp_ht()) - tiHeatCool) * rhoCpAir_277 + DBL_MIN),
                    r_Qneed_cl[i] / ((tiHeatCool - (cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl())) * rhoCpAir_277 + DBL_MIN) });

                r_Qfan_tot[i] = Vair * ventilation.fanPower() * 0.277778;
                r_Qpump_tot[i] = (r_Qneed_cl[i] > 0) ? (cooling.E_pumps() * cooling.pumpControlReduction()) :
                    ((r_Qneed_ht[i] > 0) ? (heating.E_pumps() * heating.pumpControlReduction()) : 0.0);
                r_Q_illum_ext_tot[i] = (egh[i] > 0) ? 0.0 : (lights.exteriorEnergy() * fixedExteriorLightingSchedule[h][d] * invFloorArea);
            }

            return processResults(r_Qneed_ht, r_Qneed_cl, r_Q_illum_tot, r_Q_illum_ext_tot, r_Qfan_tot, r_Qpump_tot, r_phi_plug, r_ext_equip, r_Q_dhw, aggregateByMonth);
        }

        void HourlyModel::calculateGains(int i, const std::vector<double>& curSolar, double egh_i,
            double schedIntLight, double schedIntEquip,
            double& out_phi_int, double& out_phii, double& out_lighting_tot, double& out_qSolarGain) {
            double lightingLevel = 0.0;
            out_qSolarGain = 0.0;
            const double maxIrrad = structure.irradianceForMaxShadingUse();
            const double invAreaNat = (areaNaturallyLightedRatio > 0) ? (53.0 / areaNaturallyLightedRatio) : 0.0;

            for (int k = 0; k < 9; ++k) {
                double sr = (k < 8) ? curSolar[k] : egh_i;
                double srCl = std::min(sr, maxIrrad);
                lightingLevel += invAreaNat * sr * (naturalLightRatio[k] + shadingUsePerWPerM2 * naturalLightShadeRatioReduction[k] * srCl);
                out_qSolarGain += sr * (solarRatio[k] + solarShadeRatioReduction[k] * shadingUsePerWPerM2 * srCl);
            }

            double electricForNaturalLightArea = std::max(0.0, maxRatioElectricLighting * (1.0 - lightingLevel / (elightNatural + DBL_MIN)));
            out_lighting_tot = (electricForNaturalLightArea * areaNaturallyLightedRatio + (1.0 - areaNaturallyLightedRatio) * maxRatioElectricLighting) * schedIntLight;
            out_phi_int = schedIntEquip + (out_lighting_tot * lights.elecInternalGains());
            out_phii = simSettings.phiSolFractionToAirNode() * out_qSolarGain + simSettings.phiIntFractionToAirNode() * out_phi_int;
        }

        void HourlyModel::calculateAirFlows(double windMps, double temperature, double tiHeatCool, double ventExh,
            double& out_tEnt, double& out_hei, double& out_h1) {
            double qSup = ventExh * windImpactSupplyRatio;
            double exhSup = -(qSup - ventExh);
            double tSupp = std::max(ventilation.ventPreheatDegC(), (1.0 - ventilation.heatRecoveryEfficiency()) * temperature + ventilation.heatRecoveryEfficiency() * 20.0);

            double qWind = 0.0769 * q4Pa * fastPow23(ventilation.dCp() * windMps * windMps);
            double absDT = std::max(std::fabs(temperature - tiHeatCool), 1e-5);
            double qStack = 0.0146 * q4Pa * fastPow23(0.5 * windImpactHz * absDT);
            double qSW = qStack + qWind + 1E-15;
            double qExf = std::max(0.0, std::max(qStack, qWind) - std::fabs(exhSup) * (0.5 * qStack + 0.667 * qWind / qSW));

            double qEnt = (exhSup > 0 ? exhSup : 0.0) + qExf + qSup;
            out_tEnt = (temperature * ((exhSup > 0 ? exhSup : 0.0) + qExf) + tSupp * qSup) / (qEnt + 1e-15);
            out_hei = 0.34 * qEnt;
            out_h1 = 1.0 / (1.0 / (out_hei + 1e-15) + 1.0 / H_tris);
        }

        double HourlyModel::solveThermalBalance(double temperature, double tEnt, double phii, double phi_int,
            double qSolarGain, double hei, double h1, double schedHeatSP,
            double schedCoolSP, double& TMT1, double& tiHeatCool) {
            double h2 = h1 + hwindowWperkm2;
            double h3 = 1.0 / (1.0 / h2 + 1.0 / H_ms);
            double h3_hem = 0.5 * (h3 + hem);
            double phisPhi0 = prsSolar * qSolarGain + prsInterior * phi_int;
            double phimPhi0 = prmSolar * qSolarGain + prmInterior * phi_int;
            double mid = phisPhi0 + hwindowWperkm2 * temperature + h1 * tEnt;
            double h3_h2 = h3 / h2;
            double t_phi = (h3_h2 * h1) / (hei + 1e-15);

            auto solveTi = [&](double pInput) {
                double phim = phimPhi0 + hem * temperature + h3_h2 * mid + t_phi * pInput;
                double tmt1 = (TMT1 * (Cm / 3.6 - h3_hem) + phim) / (Cm / 3.6 + h3_hem);
                double ts = (H_ms * (0.5 * (TMT1 + tmt1)) + mid + h1 * pInput / (hei + 1e-15)) / (H_ms + hwindowWperkm2 + h1);
                return (H_tris * ts + hei * tEnt + pInput) / (H_tris + hei);
                };

            double ti_0 = solveTi(phii);
            double ti_10 = solveTi(phii + 10.0);
            double denom = (ti_10 - ti_0) + 1e-15;
            double phiHC = std::max(0.0, 10.0 * (schedHeatSP - ti_0) / denom) + std::min(0.0, 10.0 * (schedCoolSP - ti_0) / denom);

            double phimFinal = phimPhi0 + hem * temperature + h3_h2 * mid + t_phi * (phii + phiHC);
            double tmt1_new = (TMT1 * (Cm / 3.6 - h3_hem) + phimFinal) / (Cm / 3.6 + h3_hem);
            double ts_final = (H_ms * (0.5 * (TMT1 + tmt1_new)) + mid + h1 * (phii + phiHC) / (hei + 1e-15)) / (H_ms + hwindowWperkm2 + h1);

            TMT1 = tmt1_new;
            tiHeatCool = (H_tris * ts_final + hei * tEnt + phii + phiHC) / (H_tris + hei);
            return phiHC;
        }

        std::vector<EndUses> HourlyModel::processResults(const std::vector<double>& r_Qneed_ht, const std::vector<double>& r_Qneed_cl,
            const std::vector<double>& r_Q_illum_tot, const std::vector<double>& r_Q_illum_ext_tot,
            const std::vector<double>& r_Qfan_tot, const std::vector<double>& r_Qpump_tot,
            const std::vector<double>& r_phi_plug, const std::vector<double>& r_ext_equip,
            const std::vector<double>& r_Q_dhw, bool aggregateByMonth) {
            double qh_yr = std::accumulate(r_Qneed_ht.begin(), r_Qneed_ht.end(), 0.0);
            double qc_yr = std::accumulate(r_Qneed_cl.begin(), r_Qneed_cl.end(), 0.0);
            double f_ht = std::max(qh_yr / (qc_yr + qh_yr + DBL_MIN), 0.1);
            double s_ht = (1.0 / (1.0 / (1.0 + heating.hvacLossFactor() + heating.hotcoldWasteFactor() / f_ht))) / heating.efficiency();
            double s_cl = (1.0 / (1.0 / (1.0 + cooling.hvacLossFactor() + heating.hotcoldWasteFactor() / (1.0 - f_ht)))) / cooling.cop();

            bool electricHeat = (heating.energyType() == 1);
            constexpr double W_TO_KWH = 0.001;
            std::vector<EndUses> results;

            auto mapToEU = [&](EndUses& eu, double h, double c, double il, double el, double fn, double pm, double pi, double pe, double dw) {
                double elec_ht = electricHeat ? (h * s_ht * W_TO_KWH) : 0.0;
                double gas_ht = electricHeat ? 0.0 : (h * s_ht * W_TO_KWH);
#ifdef ISOMODEL_STANDALONE
                eu.addEndUse(0, elec_ht); eu.addEndUse(1, c * s_cl * W_TO_KWH); eu.addEndUse(2, il * W_TO_KWH); eu.addEndUse(3, el * W_TO_KWH);
                eu.addEndUse(4, fn * W_TO_KWH); eu.addEndUse(5, pm * W_TO_KWH); eu.addEndUse(6, pi * W_TO_KWH); eu.addEndUse(7, pe * W_TO_KWH);
                eu.addEndUse(8, dw * W_TO_KWH); eu.addEndUse(9, gas_ht);
#else
                eu.addEndUse(elec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
                eu.addEndUse(c * s_cl * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
                eu.addEndUse(il * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
                eu.addEndUse(el * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
                eu.addEndUse(fn * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Fans);
                eu.addEndUse(pm * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
                eu.addEndUse(pi * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
                eu.addEndUse(pe * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
                eu.addEndUse(dw * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);
                eu.addEndUse(gas_ht, EndUseFuelType::Gas, EndUseCategoryType::Heating);
#endif
                };

            if (aggregateByMonth) {
                const int monthEnds[] = { 0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760 };
                for (int m = 0; m < 12; ++m) {
                    EndUses eu;
                    double sums[9] = { 0 };
                    for (int i = monthEnds[m]; i < monthEnds[m + 1]; ++i) {
                        sums[0] += r_Qneed_ht[i]; sums[1] += r_Qneed_cl[i]; sums[2] += r_Q_illum_tot[i]; sums[3] += r_Q_illum_ext_tot[i];
                        sums[4] += r_Qfan_tot[i]; sums[5] += r_Qpump_tot[i]; sums[6] += r_phi_plug[i]; sums[7] += r_ext_equip[i]; sums[8] += r_Q_dhw[i];
                    }
                    mapToEU(eu, sums[0], sums[1], sums[2], sums[3], sums[4], sums[5], sums[6], sums[7], sums[8]);
                    results.push_back(eu);
                }
            }
            else {
                for (int i = 0; i < 8760; ++i) {
                    EndUses eu;
                    mapToEU(eu, r_Qneed_ht[i], r_Qneed_cl[i], r_Q_illum_tot[i], r_Q_illum_ext_tot[i], r_Qfan_tot[i], r_Qpump_tot[i], r_phi_plug[i], r_ext_equip[i], r_Q_dhw[i]);
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
                structureCalculations(structure.windowShadingDevice()[i], structure.wallArea()[i],
                    structure.windowArea()[i], structure.wallUniform()[i], structure.windowUniform()[i],
                    structure.wallSolarAbsorption()[i], structure.windowShadingCorrectionFactor()[i],
                    structure.windowNormalIncidenceSolarEnergyTransmittance()[i], i);

                nlaWMovableShading[i] = nlams[i] * invFloorArea;
                naturalLightRatio[i] = nla[i] * invFloorArea;
                naturalLightShadeRatioReduction[i] = nlaWMovableShading[i] - naturalLightRatio[i];
                saWMovableShading[i] = sams[i] * invFloorArea;
                solarRatio[i] = sa[i] * invFloorArea;
                solarShadeRatioReduction[i] = saWMovableShading[i] - solarRatio[i];
            }

            shadingUsePerWPerM2 = structure.shadingFactorAtMaxUse() / structure.irradianceForMaxShadingUse();
            q4Pa = std::max(0.000001, (0.19 * (ventilation.n50() * (floorArea * structure.buildingHeight()))) * invFloorArea);

            h_ms = simSettings.hci() + simSettings.hri() * 1.2;
            h_is = 1.0 / (1.0 / simSettings.hci() - 1.0 / h_ms);
            H_tris = h_is * structure.totalAreaPerFloorArea();

            // Fix: Use std::accumulate instead of sum() for vector addition
            const auto& wallAreas = structure.wallArea();
            double totalWallArea = std::accumulate(wallAreas.begin(), wallAreas.end(), 0.0);

            Cm = (structure.interiorHeatCapacity() + (structure.wallHeatCapacity() * totalWallArea * invFloorArea)) / 1000.0;

            if (Cm > 370.0) Am = 3.5;
            else if (Cm > 260.0) Am = 3.0 + 0.5 * ((Cm - 260) / 110.0);
            else if (Cm > 165.0) Am = 2.5 + 0.5 * ((Cm - 165) / 95.0);
            else Am = 2.5;

            double hWind = 0.0, hWall = 0.0;
            for (int i = 0; i != 9; ++i) { hWind += hWindow[i]; hWall += htot[i] - hWindow[i]; }
            hwindowWperkm2 = hWind * invFloorArea;

            prs = (structure.totalAreaPerFloorArea() - Am - hwindowWperkm2 / h_ms) / structure.totalAreaPerFloorArea();
            prsInterior = (1.0 - simSettings.phiIntFractionToAirNode()) * prs;
            prsSolar = (1.0 - simSettings.phiSolFractionToAirNode()) * prs;
            prm = Am / structure.totalAreaPerFloorArea();
            prmInterior = (1.0 - simSettings.phiIntFractionToAirNode()) * prm;
            prmSolar = (1.0 - simSettings.phiSolFractionToAirNode()) * prm;

            H_ms = h_ms * Am;
            hem = 1.0 / (1.0 / std::max(hWall * invFloorArea, 0.000001) - 1.0 / H_ms);
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
                    bool popoccupied = hoccupied && (d >= dayStart && d <= dayEnd);
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
    }
}