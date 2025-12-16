/*
 * HourlyModel.cpp
 *
 * Optimized Refactor
 * Changes:
 * - INLINED calculateHour directly into simulate loop to eliminate function call overhead.
 * - Reduced register pressure by using local variables instead of pointer references.
 * - Maintained fastPow23 and preallocation improvements.
 */
/*
 * HourlyModel.cpp
 *
 * Optimized Refactor
 * Changes:
 * - REMOVED DUPLICATE definition of fastPow23 (it remains in HourlyModel.hpp).
 * - FIXED INCORRECT HOISTING of dynamic thermal constants (h1, h3 dependencies).
 */

#include "HourlyModel.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat> // DBL_MIN
#include <vector>

namespace openstudio {
namespace isomodel {

// REMOVED: inline double fastPow23(double x) { return std::cbrt(x * x); }
// It is now only defined in the .hpp file to avoid redefinition.

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

    // 1. PREALLOCATE RESULTS
    size_t numHours = 8760;
    std::vector<double> r_Qneed_ht(numHours);
    std::vector<double> r_Qneed_cl(numHours);
    std::vector<double> r_Q_illum_tot(numHours);
    std::vector<double> r_Q_illum_ext_tot(numHours);
    std::vector<double> r_Qfan_tot(numHours);
    std::vector<double> r_Qpump_tot(numHours);
    std::vector<double> r_phi_plug(numHours);
    std::vector<double> r_ext_equip(numHours);
    std::vector<double> r_Q_dhw(numHours);

    // Local constants hoisted from loop
    const double iFA = invFloorArea;
    const double maxIrrad = structure.irradianceForMaxShadingUse();
    const double invAreaNat = (areaNaturallyLightedRatio > 0) ? (53.0 / areaNaturallyLightedRatio) : 0.0;
    const double elightNaturalSafe = elightNatural + DBL_MIN; // pre-add epsilon
    const double ventFanFactor = ventilation.fanPower() * 0.277778; // 1000/3600
    
    // Hvac / Vent constants
    const double ventHeatRecEff = ventilation.heatRecoveryEfficiency();
    const double ventPreheat = ventilation.ventPreheatDegC();
    const double dCpVent = ventilation.dCp();
    const double pumpControlCl = cooling.pumpControlReduction();
    const double pumpControlHt = heating.pumpControlReduction();
    const double ePumpsCl = cooling.E_pumps();
    const double ePumpsHt = heating.E_pumps();
    const double extLightEnergy = lights.exteriorEnergy();
    const double elecInternalGains = lights.elecInternalGains();
    
    // Thermal network constants (Static parts)
    const double Cm_36 = Cm / 3.6;
    const double T_sup_ht_base = heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
    const double T_sup_cl_base = cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();
    
    // REMOVED INCORRECT HOISTED CONSTANTS HERE (h3_hem, inv_denom_Ts, etc.)
    // They are now calculated dynamically inside the loop.

    // -------------------------------------------------------------------------
    // MAIN SIMULATION LOOP (Inlined calculateHour)
    // -------------------------------------------------------------------------
    for (int i = 0; i < numHours; ++i) {
        // --- INPUTS ---
        int hourOfDay = frame.Hour[i];
        int dayOfWeek = frame.DayOfWeek[i]; // 0-6
        double windMps = wind[i];
        double temperature = temp[i];
        
        // Fast Schedule Lookup
        double schedVent      = fixedVentilationSchedule[hourOfDay][dayOfWeek];
        double schedExtEquip  = fixedExteriorEquipmentSchedule[hourOfDay][dayOfWeek];
        double schedIntEquip  = fixedInteriorEquipmentSchedule[hourOfDay][dayOfWeek];
        double schedExtLight  = fixedExteriorLightingSchedule[hourOfDay][dayOfWeek];
        double schedIntLight  = fixedInteriorLightingSchedule[hourOfDay][dayOfWeek];
        double schedHeatSP    = fixedActualHeatingSetpoint[hourOfDay][dayOfWeek];
        double schedCoolSP    = fixedActualCoolingSetpoint[hourOfDay][dayOfWeek];

        // --- CALCULATIONS ---

        // 1. Equipment & Plug
        double val_ext_equip = schedExtEquip * iFA;
        double val_phi_plug = schedIntEquip;

        // 2. Lighting & Solar
        double lightingLevel = 0.0;
        double qSolarHeatGain = 0.0;

        // Unrolled Solar Loop 
        const std::vector<double>& currentSolar = eglobe[i];
        double globalHoriz = egh[i]; // index 8

        for (int k = 0; k < 8; ++k) {
            double sr = currentSolar[k];
            double srCl = (sr < maxIrrad) ? sr : maxIrrad;
            // nla/solar arrays are member variables
            lightingLevel += invAreaNat * sr * (naturalLightRatio[k] + shadingUsePerWPerM2 * naturalLightShadeRatioReduction[k] * srCl);
            qSolarHeatGain += sr * (solarRatio[k] + solarShadeRatioReduction[k] * shadingUsePerWPerM2 * srCl);
        }
        // Roof/Horizontal (Index 8)
        {
            double sr = globalHoriz;
            double srCl = (sr < maxIrrad) ? sr : maxIrrad;
            lightingLevel += invAreaNat * sr * (naturalLightRatio[8] + shadingUsePerWPerM2 * naturalLightShadeRatioReduction[8] * srCl);
            qSolarHeatGain += sr * (solarRatio[8] + solarShadeRatioReduction[8] * shadingUsePerWPerM2 * srCl);
        }

        double electricForNaturalLightArea = std::max(0.0, maxRatioElectricLighting * (1.0 - lightingLevel / elightNaturalSafe));
        double electricForTotalLightArea = electricForNaturalLightArea * areaNaturallyLightedRatio
                                         + (1.0 - areaNaturallyLightedRatio) * maxRatioElectricLighting;

        double val_Q_illum_tot = electricForTotalLightArea * schedIntLight;
        double phi_illum = val_Q_illum_tot * elecInternalGains;
        double phi_int = val_phi_plug + phi_illum;

        // Gains to Air Node
        double phii = simSettings.phiSolFractionToAirNode() * qSolarHeatGain + simSettings.phiIntFractionToAirNode() * phi_int;
        double phii10 = phii + 10.0;

        // 3. Ventilation
        double ventExhaustM3phpm2 = schedVent * 3.6 * iFA;
        double qSupplyBySystem = ventExhaustM3phpm2 * windImpactSupplyRatio;
        double exhaustSupply = -(qSupplyBySystem - ventExhaustM3phpm2);
        
        double tAfterExchange = (1.0 - ventHeatRecEff) * temperature + ventHeatRecEff * 20.0;
        double tSuppliedAir = std::max(ventPreheat, tAfterExchange);

        // ISO 15242 Infiltration
        double term1 = dCpVent * windMps * windMps;
        double qWind = 0.0769 * q4Pa * fastPow23(term1);

        double absDT = std::fabs(temperature - tiHeatCool);
        double term2 = 0.5 * windImpactHz * ((absDT > 1e-5) ? absDT : 1e-5);
        double qStackPrevIntTemp = 0.0146 * q4Pa * fastPow23(term2);

        double qExfiltration = 0.0;
        double qStackPlusWind = qStackPrevIntTemp + qWind;
        if (qStackPlusWind > 1e-9) {
            double subTerm = 0.5 * qStackPrevIntTemp + 0.667 * qWind / qStackPlusWind;
            double diff = std::max(qStackPrevIntTemp, qWind) - std::fabs(exhaustSupply) * subTerm;
            if (diff > 0) qExfiltration = diff;
        }

        double qEnvelope = std::max(0.0, exhaustSupply) + qExfiltration;
        double qEnteringTotal = qEnvelope + qSupplyBySystem;

        double tEnteringAndSupplied = temperature;
        if (qEnteringTotal > 1e-9) {
            tEnteringAndSupplied = (temperature * qEnvelope + tSuppliedAir * qSupplyBySystem) / qEnteringTotal;
        }

        // 4. Thermal Network Dynamic Calc (Inside the loop where it belongs)
        double hei = 0.34 * qEnteringTotal; // H_{tr,e}
        double h1_dyn = 1.0 / (1.0 / hei + 1.0 / H_tris); // H_{tr,1}
        double h2_dyn = h1_dyn + hwindowWperkm2; // H_{tr,2}
        double h3_dyn = 1.0 / (1.0 / h2_dyn + 1.0 / H_ms); // H_{tr,3}

        double phisPhi0 = prsSolar * qSolarHeatGain + prsInterior * phi_int;
        double phimPhi0 = prmSolar * qSolarHeatGain + prmInterior * phi_int;

        // Composite terms (Calculated dynamically here)
        double h3_hem_dyn = 0.5 * (h3_dyn + hem); // Uses dynamic h3_dyn
        double inv_denom_Tm_dyn = 1.0 / (Cm_36 + h3_hem_dyn);
        double inv_denom_Ts_dyn = 1.0 / (H_ms + hwindowWperkm2 + h1_dyn); // Uses dynamic h1_dyn
        double inv_denom_Ti_dyn = 1.0 / (H_tris + hei);

        double midTermCommon = phisPhi0 + hwindowWperkm2 * temperature + h1_dyn * tEnteringAndSupplied;
        double h3_h2_dyn = h3_dyn / h2_dyn;
        double term_phi_dyn = h3_h2_dyn * h1_dyn / hei;

        // Step 1 (Perturbation 10W)
        double phimTotal10 = phimPhi0 + hem * temperature + h3_h2_dyn * midTermCommon + term_phi_dyn * phii10;
        double tmt1_10 = (TMT1 * (Cm_36 - h3_hem_dyn) + phimTotal10) * inv_denom_Tm_dyn;
        double tm_10 = 0.5 * (TMT1 + tmt1_10);
        double ts_10 = (H_ms * tm_10 + midTermCommon + h1_dyn * phii10 / hei) * inv_denom_Ts_dyn;
        double ti_10 = (H_tris * ts_10 + hei * tEnteringAndSupplied + phii10) * inv_denom_Ti_dyn;

        // Step 2 (No perturbation)
        double phimTotal0 = phimPhi0 + hem * temperature + h3_h2_dyn * midTermCommon + term_phi_dyn * phii;
        double tmt1_0 = (TMT1 * (Cm_36 - h3_hem_dyn) + phimTotal0) * inv_denom_Tm_dyn;
        double tm_0 = 0.5 * (TMT1 + tmt1_0);
        double ts_0 = (H_ms * tm_0 + midTermCommon + h1_dyn * phii / hei) * inv_denom_Ts_dyn;
        double ti_0 = (H_tris * ts_0 + hei * tEnteringAndSupplied + phii) * inv_denom_Ti_dyn;

        // Demand
        double val_Qneed_cl = 0.0;
        double val_Qneed_ht = 0.0;
        double denom = ti_10 - ti_0;
        double phiHC = 0.0;
        
        if (std::abs(denom) > 1e-9) {
            double phiCool = 10.0 * (schedCoolSP - ti_0) / denom;
            double phiHeat = 10.0 * (schedHeatSP - ti_0) / denom;
            phiHC = std::max(0.0, phiHeat) + std::min(phiCool, 0.0);
            val_Qneed_ht = std::max(0.0, phiHC);
            val_Qneed_cl = std::max(0.0, -phiHC);
        }

        // Update State
        double phiiActual = phii + phiHC;
        double phimTotalActual = phimPhi0 + hem * temperature + h3_h2_dyn * midTermCommon + term_phi_dyn * phiiActual;
        
        double tmt1_new = (TMT1 * (Cm_36 - h3_hem_dyn) + phimTotalActual) * inv_denom_Tm_dyn;
        TMT1 = tmt1_new; // STATE UPDATE

        double tm_new = 0.5 * (TMT1 + tmt1_new); // uses new TMT1
        double ts_new = (H_ms * tm_new + midTermCommon + h1_dyn * phiiActual / hei) * inv_denom_Ts_dyn;
        tiHeatCool = (H_tris * ts_new + hei * tEnteringAndSupplied + phiiActual) * inv_denom_Ti_dyn; // STATE UPDATE

        // 5. Fans & Pumps
        double Vair_tot = ventExhaustM3phpm2;
        if (val_Qneed_ht > 0) {
            Vair_tot = std::max(Vair_tot, val_Qneed_ht / ((T_sup_ht_base - tiHeatCool) * rhoCpAir_277 + DBL_MIN));
        }
        if (val_Qneed_cl > 0) {
            Vair_tot = std::max(Vair_tot, val_Qneed_cl / ((tiHeatCool - T_sup_cl_base) * rhoCpAir_277 + DBL_MIN));
        }
        double val_Qfan_tot = Vair_tot * ventFanFactor;

        double val_Qpump_tot = 0.0;
        if (val_Qneed_cl > 0) val_Qpump_tot = ePumpsCl * pumpControlCl;
        else if (val_Qneed_ht > 0) val_Qpump_tot = ePumpsHt * pumpControlHt;

        double val_Q_illum_ext_tot = (globalHoriz > 0) ? 0.0 : (extLightEnergy * schedExtLight * iFA);

        // --- WRITE OUTPUTS (Batch Write) ---
        r_ext_equip[i]      = val_ext_equip;
        r_phi_plug[i]       = val_phi_plug;
        r_Q_illum_tot[i]    = val_Q_illum_tot;
        r_Qneed_ht[i]       = val_Qneed_ht;
        r_Qneed_cl[i]       = val_Qneed_cl;
        r_Qfan_tot[i]       = val_Qfan_tot;
        r_Qpump_tot[i]      = val_Qpump_tot;
        r_Q_illum_ext_tot[i]= val_Q_illum_ext_tot;
        r_Q_dhw[i]          = 0.0;
    }

    // --- Post Processing (Same as before) ---

    // Constants for aggregations
    double sys_factor_ht = 0.0;
    double sys_factor_cl = 0.0;
    {
        double Qneed_ht_yr = std::accumulate(r_Qneed_ht.begin(), r_Qneed_ht.end(), 0.0);
        double Qneed_cl_yr = std::accumulate(r_Qneed_cl.begin(), r_Qneed_cl.end(), 0.0);
        
        double f_dem_ht = std::max(Qneed_ht_yr / (Qneed_cl_yr + Qneed_ht_yr + DBL_MIN), 0.1);
        double f_dem_cl = std::max((1.0 - f_dem_ht), 0.1);
        
        double eta_dist_ht = 1.0 / (1.0 + heating.hvacLossFactor() + heating.hotcoldWasteFactor() / f_dem_ht);
        double eta_dist_cl = 1.0 / (1.0 + cooling.hvacLossFactor() + heating.hotcoldWasteFactor() / f_dem_cl);

        sys_factor_ht = 1.0 / eta_dist_ht / heating.efficiency();
        sys_factor_cl = 1.0 / eta_dist_cl / cooling.cop();
    }

    bool electricHeat = (heating.energyType() == 1);
    constexpr double W_TO_KWH = 0.001;
    
    std::vector<EndUses> results;
    
    if (aggregateByMonth) {
        results.reserve(12);
        const int monthEnds[] = {0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760};

        for (int m = 0; m < 12; ++m) {
            double sum_sys_ht = 0, sum_sys_cl = 0, sum_illum_int = 0, sum_illum_ext = 0;
            double sum_fan = 0, sum_pump = 0, sum_plug_int = 0, sum_plug_ext = 0, sum_dhw = 0;

            for (int h = monthEnds[m]; h < monthEnds[m+1]; ++h) {
                sum_sys_ht    += r_Qneed_ht[h];
                sum_sys_cl    += r_Qneed_cl[h];
                sum_illum_int += r_Q_illum_tot[h];
                sum_illum_ext += r_Q_illum_ext_tot[h];
                sum_fan       += r_Qfan_tot[h];
                sum_pump      += r_Qpump_tot[h];
                sum_plug_int  += r_phi_plug[h];
                sum_plug_ext  += r_ext_equip[h];
                sum_dhw       += r_Q_dhw[h];
            }

            double elec_ht = electricHeat ? (sum_sys_ht * sys_factor_ht * W_TO_KWH) : 0.0;
            double gas_ht  = electricHeat ? 0.0 : (sum_sys_ht * sys_factor_ht * W_TO_KWH);

            EndUses eu;
#ifdef ISOMODEL_STANDALONE
            // ... (EndUses logic) ...
            eu.addEndUse(0, elec_ht);
            eu.addEndUse(1, sum_sys_cl * sys_factor_cl * W_TO_KWH);
            eu.addEndUse(2, sum_illum_int * W_TO_KWH);
            eu.addEndUse(3, sum_illum_ext * W_TO_KWH);
            eu.addEndUse(4, sum_fan * W_TO_KWH);
            eu.addEndUse(5, sum_pump * W_TO_KWH);
            eu.addEndUse(6, sum_plug_int * W_TO_KWH);
            eu.addEndUse(7, sum_plug_ext * W_TO_KWH);
            eu.addEndUse(8, sum_dhw * W_TO_KWH);
            eu.addEndUse(9, gas_ht);
#else
            // ... (EndUses logic) ...
            eu.addEndUse(elec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
            eu.addEndUse(sum_sys_cl * sys_factor_cl * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
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
    } else {
        // Hourly Aggregation (unchanged)
        results.reserve(8760);
        for (int i = 0; i < 8760; ++i) {
            EndUses eu;
            double elec_ht = electricHeat ? (r_Qneed_ht[i] * sys_factor_ht * W_TO_KWH) : 0.0;
            double gas_ht  = electricHeat ? 0.0 : (r_Qneed_ht[i] * sys_factor_ht * W_TO_KWH);
#ifdef ISOMODEL_STANDALONE
            // ... (EndUses logic) ...
            eu.addEndUse(0, elec_ht);
            eu.addEndUse(1, r_Qneed_cl[i] * sys_factor_cl * W_TO_KWH);
            eu.addEndUse(2, r_Q_illum_tot[i] * W_TO_KWH);
            eu.addEndUse(3, r_Q_illum_ext_tot[i] * W_TO_KWH);
            eu.addEndUse(4, r_Qfan_tot[i] * W_TO_KWH);
            eu.addEndUse(5, r_Qpump_tot[i] * W_TO_KWH);
            eu.addEndUse(6, r_phi_plug[i] * W_TO_KWH);
            eu.addEndUse(7, r_ext_equip[i] * W_TO_KWH);
            eu.addEndUse(8, r_Q_dhw[i] * W_TO_KWH);
            eu.addEndUse(9, gas_ht);
#else
            // ... (EndUses logic) ...
            eu.addEndUse(elec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
            eu.addEndUse(r_Qneed_cl[i] * sys_factor_cl * W_TO_KWH, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
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
    // ... (Initialization Logic - Unchanged) ...
    double floorArea = structure.floorArea();
    invFloorArea = (floorArea > 0) ? 1.0/floorArea : 0.0;
    rhoCpAir_277 = phys.rhoCpAir() * 277.777778;

    auto lightingOccupancySensorDimmingFraction = building.lightingOccupancySensor();
    auto daylightSensorDimmingFraction = lights.dimmingFraction();

    if (lightingOccupancySensorDimmingFraction < 1.0 && daylightSensorDimmingFraction < 1.0) {
        maxRatioElectricLighting = lights.presenceAutoAd();
        elightNatural = lights.presenceAutoLux();
    } else if (lightingOccupancySensorDimmingFraction < 1.0) {
        maxRatioElectricLighting = lights.presenceSensorAd();
        elightNatural = lights.presenceSensorLux();
    } else if (daylightSensorDimmingFraction < 1.0) {
        maxRatioElectricLighting = lights.automaticAd();
        elightNatural = lights.automaticLux();
    } else {
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
    
    // ISO 15242 Air leakage
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

    double hWind = 0.0;
    double hWall = 0.0;
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
    // ... (Schedule Logic - Unchanged) ...
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
    // ... (Structure Logic - Unchanged) ...
    double WindowT = SHGC / 0.87;
    nlams[direction] = windowAreaM2 * WindowT; 
    nla[direction] = windowAreaM2 * WindowT; 
    sams[direction] = wallAreaM2 * (wallSolarAbsorption * wallUValue * structure.R_se()) + windowAreaM2 * solarFactorWith;
    sa[direction] = wallAreaM2 * (wallSolarAbsorption * wallUValue * structure.R_se()) + windowAreaM2 * solarFactorWithout;
    htot[direction] = wallAreaM2 * wallUValue + windowAreaM2 * windowUValue;
    hWindow[direction] = windowAreaM2 * windowUValue;
}

std::vector<double> HourlyModel::sumHoursByMonth(const std::vector<double>& hourlyData) { return {}; }

} // namespace
} // namespace