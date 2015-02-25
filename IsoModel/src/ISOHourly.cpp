/*
 * ISOHourly.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: craig
 */

// Some notes on the comments:
// In comments, symbols and formulas from the standard are written in LaTeX
// markup to reduce ambiguity.
//
// While this is a work in progress, comments relating the code to the cells of
// the spreadsheet on which this is based will be retained (e.g.,
// SingleBldg.L50).

#include "ISOHourly.hpp"
#include "SimModel.hpp"

#include <numeric>
#include <algorithm>
#include <vector>
#include <functional>
#include <iterator>
#include <string>
#include <map>

namespace openstudio {
namespace isomodel {

//TODO This initializer list should be removed and these attributes included in the ism file. -BAA@2014-12-14
ISOHourly::ISOHourly() : electInternalGains(1), // SingleBldg.L51
                         permLightPowerDensityWperM2(0), // SingleBldg.L50
                         externalEquipment(0), // Used to have a hardcoded value of 244000. Set to 0 until it gets added as an ism attribute. Q56
                         ventPreheatDegC(-50) // SingleBldg.Q40
{
}

ISOHourly::~ISOHourly()
{
}

ISOResults ISOHourly::calculateHourly(bool aggregateByMonth)
{
  populateSchedules();

  // printMatrix("Cooling Setpoint", (double*) this->fixedActualCoolingSetpoint, 24, 7);
  // printMatrix("Heating Setpoint", (double*) this->fixedActualHeatingSetpoint, 24, 7);
  // printMatrix("Exterior Equipment", (double*) this->fixedExteriorEquipmentSchedule, 24, 7);
  // printMatrix("Exterior Lighting", (double*) this->fixedExteriorLightingSchedule, 24, 7);
  // printMatrix("Fan", (double*) this->fixedFanSchedule, 24, 7);
  // printMatrix("Interior Equipment", (double*) this->fixedInteriorEquipmentSchedule, 24, 7);
  // printMatrix("Interior Lighting", (double*) this->fixedInteriorLightingSchedule, 24, 7);
  // printMatrix("Ventilation", (double*) this->fixedVentilationSchedule, 24, 7);

  initialize();
  int hourOfDay = 1;
  int dayOfWeek = 1;
  int month = 1;
  TimeFrame frame;
  double TMT1, tiHeatCool;
  TMT1 = tiHeatCool = 20;
  std::vector<double> wind = weatherData->data()[WSPD];
  std::vector<double> temp = weatherData->data()[DBT];
  SolarRadiation pos(&frame, weatherData.get());
  pos.Calculate();
  std::vector<std::vector<double> > radiation = pos.eglobe(); // Radiation for 8 directions (N, NE, E, etc.).
  std::vector<double> globalHorizontalRadiation = weatherData->data()[EGH]; // Radiation for roof.
  HourResults<double> tempHourResults;
  HourResults<std::vector<double>> rawResults;

  for (int i = 0; i < TIMESLICES; i++) {
    month = frame.Month[i];
    if (hourOfDay == 25) {
      hourOfDay = 1;
      dayOfWeek = (dayOfWeek == 7) ? 1 : dayOfWeek + 1;
    }
    
    calculateHour(i + 1, //hourOfYear
                  month, //month
                  dayOfWeek, //dayOfWeek
                  hourOfDay, //hourOfDay
                  wind[i], //windMps
                  temp[i], //temperature
                  radiation[i][0],
                  radiation[i][2],
                  radiation[i][4],
                  radiation[i][6],
                  globalHorizontalRadiation[i],
                  TMT1, //TMT1
                  tiHeatCool, //tiHeatCool
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

    // Increment the hour.
    ++hourOfDay;
  }

  // Factor the raw need results by the distribution efficiencies.
  double a_ht_loss = heating->hvacLossFactor();
  double a_cl_loss = cooling->hvacLossFactor();
  double f_waste = heating->hotcoldWasteFactor();
  double cop = cooling->cop();
  double efficiency_ht = heating->efficiency();

  // Calculate the yearly totals.
  double Qneed_ht_yr = std::accumulate(rawResults.Qneed_ht.begin(), rawResults.Qneed_ht.end(), 0.0);
  double Qneed_cl_yr = std::accumulate(rawResults.Qneed_cl.begin(), rawResults.Qneed_cl.end(), 0.0);

  double f_dem_ht = std::max(Qneed_ht_yr / (Qneed_cl_yr + Qneed_ht_yr), 0.1);
  double f_dem_cl = std::max((1.0 - f_dem_ht), 0.1);

  double eta_dist_ht = 1.0 / (1.0 + a_ht_loss + f_waste / f_dem_ht);
  double eta_dist_cl = 1.0 / (1.0 + a_cl_loss + f_waste / f_dem_cl);

  // Create unary functions to factor the heating and cooling values for use
  // with transform().
  auto factorHeating = [=](double need) {return need / eta_dist_ht / efficiency_ht;};
  auto factorCooling = [=](double need) {return need / eta_dist_cl / cop;};

  // Create containers for factored heating and cooling values.
  std::vector<double> v_Qht_sys(rawResults.Qneed_ht.size());
  std::vector<double> v_Qcl_sys(rawResults.Qneed_cl.size());

  // Factor the heating and cooling values.
  std::transform(rawResults.Qneed_ht.begin(), rawResults.Qneed_ht.end(), v_Qht_sys.begin(), factorHeating);
  std::transform(rawResults.Qneed_cl.begin(), rawResults.Qneed_cl.end(), v_Qcl_sys.begin(), factorCooling);

  // Store the factored results and rename them to match the monthly result names.
  std::map<std::string, std::vector<double> > results;
  
  // TODO Fix this! Hardcoded values of '0' for things not being calculated is not ideal.
  std::vector<double> zeroes(rawResults.Qneed_ht.size(), 0.0);

  results["Eelec_ht"] = (heating->energyType() == 1) ? v_Qht_sys : zeroes; // If electric.
  results["Eelec_cl"] = v_Qcl_sys;
  results["Eelec_int_lt"] = rawResults.Q_illum_tot;
  results["Eelec_ext_lt"] = rawResults.Q_illum_ext_tot;
  results["Eelec_fan"] = rawResults.Qfan_tot;
  results["Eelec_pump"] = rawResults.Qpump_tot;
  results["Eelec_int_plug"] = rawResults.phi_plug;
  results["Eelec_ext_plug"] = rawResults.externalEquipmentEnergyWperm2; // TODO BAA@2015-01-28. This is currently hardcoded and shouldn't be.
  results["Eelec_dhw"] = rawResults.Q_dhw;
  results["Egas_ht"] = (heating->energyType() != 1) ? v_Qht_sys : zeroes; // If not electric.
  results["Egas_cl"] = zeroes;
  results["Egas_plug"] = zeroes;
  results["Egas_dhw"] = zeroes;

  // Convert to EUI in kWh/m^2
  auto wattsPerHourTokWh = [](double watts) {return watts / 1000.0;};

  for (auto& kv : results) {
    std::transform(kv.second.begin(), kv.second.end(), kv.second.begin(), wattsPerHourTokWh);
  }

  if (aggregateByMonth) {
    // Calculate monthly results
    for (const auto& kv : results) {
      results[kv.first] = sumHoursByMonth(kv.second);
    }
  }

  int numberOfResults = aggregateByMonth ? 12 : 8760;

  ISOResults allResults;
  for (int i = 0; i < numberOfResults; i++) {
    EndUses timestepEndUses;
#ifdef _OPENSTUDIOS
    timestepEndUses.addEndUse(results["Eelec_ht"][i], EndUseFuelType::Electricity, EndUseCategoryType::Heating);
    timestepEndUses.addEndUse(results["Eelec_cl"][i], EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
    timestepEndUses.addEndUse(results["Eelec_int_lt"][i], EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
    timestepEndUses.addEndUse(results["Eelec_ext_lt"][i], EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
    timestepEndUses.addEndUse(results["Eelec_fan"][i], EndUseFuelType::Electricity, EndUseCategoryType::Fans);
    timestepEndUses.addEndUse(results["Eelec_pump"][i], EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
    timestepEndUses.addEndUse(results["Eelec_int_plug"][i], EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
    timestepEndUses.addEndUse(results["Eelec_ext_plug"][i], EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
    timestepEndUses.addEndUse(results["Eelec_dhw"][i], EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);

    timestepEndUses.addEndUse(results["Egas_ht"][i], EndUseFuelType::Gas, EndUseCategoryType::Heating);
    timestepEndUses.addEndUse(results["Egas_cl"][i], EndUseFuelType::Gas, EndUseCategoryType::Cooling);
    timestepEndUses.addEndUse(results["Egas_plug"][i], EndUseFuelType::Gas, EndUseCategoryType::InteriorEquipment);
    timestepEndUses.addEndUse(results["Egas_dhw"][i], EndUseFuelType::Gas, EndUseCategoryType::WaterSystems);
#else
    int euse = 0;
    timestepEndUses.addEndUse(euse++, results["Eelec_ht"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_cl"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_int_lt"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_ext_lt"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_fan"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_pump"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_int_plug"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_ext_plug"][i]);
    timestepEndUses.addEndUse(euse++, results["Eelec_dhw"][i]);
    timestepEndUses.addEndUse(euse++, results["Egas_ht"][i]);
    timestepEndUses.addEndUse(euse++, results["Egas_cl"][i]);
    timestepEndUses.addEndUse(euse++, results["Egas_plug"][i]);
    timestepEndUses.addEndUse(euse++, results["Egas_dhw"][i]);
#endif
    allResults.hourlyResults.push_back(timestepEndUses);
  }
  return allResults;
}

void ISOHourly::calculateHour(int hourOfYear,
                              int month,
                              int dayOfWeek,
                              int hourOfDay,
                              double windMps,
                              double temperature,
                              double solarRadiationS,
                              double solarRadiationE,
                              double solarRadiationN,
                              double solarRadiationW,
                              double solarRadiationH,
                              double& TMT1,
                              double& tiHeatCool,
                              HourResults<double>& results)
{
  int scheduleOffset = (dayOfWeek % 7) == 0 ? 7 : dayOfWeek % 7; // ExcelFunctions.printOut("E156",scheduleOffset,1);

  // Extract schedules to a function so that we can populate them based on
  // timeslice instead of fixed schedules.
  double fanEnabled = fanSchedule(hourOfYear, hourOfDay, scheduleOffset); //ExcelFunctions.OFFSET(CZ90,hourOfDay-1,E156-1)
  // Convert ventilation from L/s to m^3/h and divide by floor area.
  double ventExhaustM3phpm2 = ventilationSchedule(hourOfYear, hourOfDay, scheduleOffset) * 3.6 / structure->floorArea(); //ExcelFunctions.OFFSET(AB90,hourOfDay-1,E156-1)
  double externalEquipmentEnabled = exteriorEquipmentSchedule(hourOfYear, hourOfDay, scheduleOffset); //ExcelFunctions.OFFSET(BV90,hourOfDay-1,E156-1)
  double internalEquipmentEnabled = interiorEquipmentSchedule(hourOfYear, hourOfDay, scheduleOffset); //ExcelFunctions.OFFSET(AK90,hourOfDay-1,E156-1)
  double exteriorLightingEnabled = exteriorLightingSchedule(hourOfYear, hourOfDay, scheduleOffset); 
  double internalLightingEnabled = interiorLightingSchedule(hourOfYear, hourOfDay, scheduleOffset); //ExcelFunctions.OFFSET(BL90,hourOfDay-1,E156-1)
  double actualHeatingSetpoint = heatingSetpointSchedule(hourOfYear, hourOfDay, scheduleOffset);
  double actualCoolingSetpoint = coolingSetpointSchedule(hourOfYear, hourOfDay, scheduleOffset);

  results.externalEquipmentEnergyWperm2 = externalEquipmentEnabled * externalEquipment / structure->floorArea();

  // \Phi_{int,A}, ISO 13790 10.4.2.
  // Monthly name: phi_plug_occ and phi_plug_unocc.
  results.phi_plug = internalEquipmentEnabled * building->electricApplianceHeatGainOccupied();

  // TODO BAA@2014-12-14: if the lightingContribution, solarRadiation,
  // naturalLightRatio and naturalLightShadeRatioReduction were all vectors,
  // this would be much simpler. 
  double lightingContributionH = 53 / areaNaturallyLightedRatio * solarRadiationH * (naturalLightRatioH + shadingUsePerWPerM2 * naturalLightShadeRatioReductionH * std::min(shadingRatioWtoM2, solarRadiationH));
  double lightingContributionW = 53 / areaNaturallyLightedRatio * solarRadiationW * (naturalLightRatioW + shadingUsePerWPerM2 * naturalLightShadeRatioReductionW * std::min(shadingRatioWtoM2, solarRadiationW));
  double lightingContributionS = 53 / areaNaturallyLightedRatio * solarRadiationS * (naturalLightRatioS + shadingUsePerWPerM2 * naturalLightShadeRatioReductionS * std::min(shadingRatioWtoM2, solarRadiationS));
  double lightingContributionE = 53 / areaNaturallyLightedRatio * solarRadiationE * (naturalLightRatioE + shadingUsePerWPerM2 * naturalLightShadeRatioReductionE * std::min(shadingRatioWtoM2, solarRadiationE));
  double lightingContributionN = 53 / areaNaturallyLightedRatio * solarRadiationN * (naturalLightRatioN + shadingUsePerWPerM2 * naturalLightShadeRatioReductionN * std::min(shadingRatioWtoM2, solarRadiationN));

  double lightingLevel = (lightingContributionN + lightingContributionE + lightingContributionS + lightingContributionW + lightingContributionH);
  double electricForNaturalLightArea = std::max(0.0, maxRatioElectricLighting * (1 - lightingLevel / elightNatural));
  double electricForTotalLightArea = electricForNaturalLightArea * areaNaturallyLightedRatio + (1 - areaNaturallyLightedRatio) * maxRatioElectricLighting;

  // Heat produced by lighting.
  // \Phi_{int,L}, ISO 13790 10.4.3. 
  // Monthly name: phi_illum_occ, phi_illum_unocc
  double phi_illum = electricForTotalLightArea * lights->powerDensityOccupied() * internalLightingEnabled * electInternalGains;

  // TODO: permLightPowerDensityWperM2 is unused.
  results.Q_illum_tot = electricForTotalLightArea * lights->powerDensityOccupied() * internalLightingEnabled;

  // \Phi_{int}, ISO 13790 10.2.2 eq. 35.
  // Monthly name: phi_int_wk_nt, phi_int_wke_day, phi_int_wke_nt.
  double phi_int = results.phi_plug + phi_illum; //1.753

  //TODO -- expand solar heat calculations to array format to include
  //diagonals. If the directional values were vectors, it would simplify things
  //considerably.
  // \Phi_{sol,k}, ISO 13790 11.3.2 eq. 43. 
  // Note: method of calculating A_{sol,k} with movable shading differs from
  // the method in the standard.
  double solarHeatGainH = solarRadiationH * (solarRatioH + solarShadeRatioReductionH * shadingUsePerWPerM2 * std::min(solarRadiationH, shadingRatioWtoM2));
  double solarHeatGainW = solarRadiationW * (solarRatioW + solarShadeRatioReductionW * shadingUsePerWPerM2 * std::min(solarRadiationW, shadingRatioWtoM2));
  double solarHeatGainS = solarRadiationS * (solarRatioS + solarShadeRatioReductionS * shadingUsePerWPerM2 * std::min(solarRadiationS, shadingRatioWtoM2));
  double solarHeatGainE = solarRadiationE * (solarRatioE + solarShadeRatioReductionE * shadingUsePerWPerM2 * std::min(solarRadiationE, shadingRatioWtoM2));
  double solarHeatGainN = solarRadiationN * (solarRatioN + solarShadeRatioReductionN * shadingUsePerWPerM2 * std::min(solarRadiationN, shadingRatioWtoM2));

  // \Phi_{sol}, ISO 13790 11.2.2 eq. 41.
  double qSolarHeatGain = (solarHeatGainN + solarHeatGainE + solarHeatGainS + solarHeatGainW + solarHeatGainH);
  // \Phi_{ia}, ISO 13790 C.2 eq. C.1. 
  // (Note that solarPair = 0 and intPair = 0.5).
  double phii = solarPair * qSolarHeatGain + intPair * phi_int;
  // \Phi_{ia10}, ISO 13790 C.4.2. 
  // Used to calculate \theta_{air,ac} when available heating or cooling power
  // is insufficient to achieve the setpoint. Adding 10 is equivalent to
  // applying 10 W/m^2 to the building because all the values in this
  // implementation are expressed per area (so as to get final results in EUI).
  double phii10 = phii + 10;

  // Ventilation from wind. ISO 15242.
  double qSupplyBySystem = ventExhaustM3phpm2 * windImpactSupplyRatio;
  double exhaustSupply = -(qSupplyBySystem - ventExhaustM3phpm2); // ISO 15242 q_{v-diff}.
  double tAfterExchange = (1 - ventilation->heatRecoveryEfficiency()) * temperature + ventilation->heatRecoveryEfficiency() * 20;
  double tSuppliedAir = std::max(ventPreheatDegC, tAfterExchange);
  // ISO 15242 6.7.1 Step 1.
  double qWind = 0.0769 * q4Pa * std::pow((ventDcpWindImpact * windMps * windMps), 0.667);
  double qStackPrevIntTemp = 0.0146 * q4Pa * std::pow((0.5 * windImpactHz * (std::max(0.00001, std::abs(temperature - tiHeatCool)))), 0.667);
  // ISO 15242 6.7.1 Step 2.
  double qExfiltration = std::max(0.0,
      std::max(qStackPrevIntTemp, qWind) - std::abs(exhaustSupply) * (0.5 * qStackPrevIntTemp + 0.667 * (qWind) / (qStackPrevIntTemp + qWind)));
  double qEnvelope = std::max(0.0, exhaustSupply) + qExfiltration;
  // ISO 15242 6.7.2.
  double qEnteringTotal = qEnvelope + qSupplyBySystem;

  // \theta_{sup} ISO 13790 9.3.
  double tEnteringAndSupplied = (temperature * qEnvelope + tSuppliedAir * qSupplyBySystem) / qEnteringTotal;
  // I think hei is H_{ve,adj} or H_{ve} ISO 13790 9.3.1 eq. 21. I'm not sure
  // what the 0.34 is.
  double hei = 0.34 * qEnteringTotal;
  // H_{tr,1}, ISO 13790 C.3 eq. C.6.
  double h1 = 1 / (1 / hei + 1 / his);
  // H_{tr,2}, ISO 13790 C.3 eq. C.7.
  double h2 = h1 + hwindowWperkm2;
  //ExcelFunctions.printOut("h2",h2,0.726440377838674);

  // Subscript '0' indicates the free-floating condition and sub '10' indicates
  // the the condition after applying 10 W/m^s. This procedure is outlined in
  // ISO 13790 C.4.2 and is used to calculate the temperature when insuficient
  // heating or cooling power is available to get the temp between the heating
  // and cooling setpoints.

  // \Phi_{st}, ISO 13790 C.2 eq. C.3 
  // In generalized form from Georgia Tech spreadsheet.
  double phisPhi0 = prsSolar * qSolarHeatGain + prsInterior * phi_int;
  // \Phi_{m}, ISO 13790 C.2 eq. C.2.
  // In generalized form from Georgia Tech spreadsheet.
  double phimPhi0 = prmSolar * qSolarHeatGain + prmInterior * phi_int;
  // H_{tr,3}, ISO 13790 C.3 eq. C.9.
  double h3 = 1 / (1 / h2 + 1 / hms);
  // \Phi_{mtot}, ISO 13790 C.3 eq. C.5.
  double phimTotalPhi10 = phimPhi0 + hem * temperature
      + h3 * (phisPhi0 + hwindowWperkm2 * temperature + h1 * (phii10 / hei + tEnteringAndSupplied)) / h2;
  double phimTotalPhi0 = phimPhi0 + hem * temperature
      + h3 * (phisPhi0 + hwindowWperkm2 * temperature + h1 * (phii / hei + tEnteringAndSupplied)) / h2;
      // \theta_{m,t10}, ISO 13790 C.3 eq. C.4.
  double tmt1Phi10 = (TMT1 * (Cm / 3.6 - 0.5 * (h3 + hem)) + phimTotalPhi10) / (Cm / 3.6 + 0.5 * (h3 + hem));
  double tmPhi10 = 0.5 * (TMT1 + tmt1Phi10);
  double tsPhi10 = (hms * tmPhi10 + phisPhi0 + hwindowWperkm2 * temperature + h1 * (tEnteringAndSupplied + phii10 / hei))
      / (hms + hwindowWperkm2 + h1);
  //ExcelFunctions.printOut("BA156",tsPhi10,19.8762155145252);
  double tiPhi10 = (his * tsPhi10 + hei * tEnteringAndSupplied + phii10) / (his + hei);
  // \theta_{m,t}, ISO 13790 C.3 eq. C.4.
  double tmt1Phi0 = (TMT1 * (Cm / 3.6 - 0.5 * (h3 + hem)) + phimTotalPhi0) / (Cm / 3.6 + 0.5 * (h3 + hem));
  double tmPhi0 = 0.5 * (TMT1 + tmt1Phi0);
  double tsPhi0 = (hms * tmPhi0 + phisPhi0 + hwindowWperkm2 * temperature + h1 * (tEnteringAndSupplied + phii / hei)) / (hms + hwindowWperkm2 + h1);
  double tiPhi0 = (his * tsPhi0 + hei * tEnteringAndSupplied + phii) / (his + hei);
  double phiCooling = 10 * (actualCoolingSetpoint - tiPhi0) / (tiPhi10 - tiPhi0);
  double phiHeating = 10 * (actualHeatingSetpoint - tiPhi0) / (tiPhi10 - tiPhi0);
  double phiActual = std::max(0.0, phiHeating) + std::min(phiCooling, 0.0);
  results.Qneed_cl = std::max(0.0, -phiActual); // Raw need. Not adjusted for efficiency.
  results.Qneed_ht = std::max(0.0, phiActual); // Raw need. Not adjusted for efficiency.
  
  // Fan power
  auto n_dT_supp_ht = 7.0; //% set heating temp diff between supply air and room air
  auto n_dT_supp_cl = 7.0; //%set cooling temp diff between supply air and room air
  auto T_sup_ht = heating->temperatureSetPointOccupied() + n_dT_supp_ht; //%hot air supply temp  - assume supply air is 7C hotter than room
  auto T_sup_cl = cooling->temperatureSetPointOccupied() - n_dT_supp_cl; //%cool air supply temp - assume 7C lower than room
  auto n_rhoC_a = 1.22521 * 0.001012 * 277.777778; // First two numbers give rho*Cp for air in MJ/m3/K, last number converts to watt-hr/m3/K.

  auto ventFanPower = ventExhaustM3phpm2 * fanEnabled;
  // XXX In the unlikely event that (T_sup_ht - TMT1) * n_rhoC_a was equal to -DBL_MIN, would this divide by zero? - BAA@2015-02-18.
  auto Vair_ht = results.Qneed_ht / (((T_sup_ht - TMT1) * n_rhoC_a) + DBL_MIN);
  auto Vair_cl = results.Qneed_cl / (((TMT1 - T_sup_cl) * n_rhoC_a) + DBL_MIN);

  auto Vair_tot = std::max((Vair_ht + Vair_cl), ventFanPower);

  // Calculate fan energy in W/m2. Air volumes in m3/h/m2, fan power in W/(L/s). Convert with (m^3 / 1000 L) * (3600 s / h)
  results.Qfan_tot = Vair_tot * ventilation->fanPower() * 1000.0 / 3600.0;

  // Determine pump energy by using the fixed pump power of .25 W/m2 if the heating
  // or cooling system is active, 0.0 if not. The .25 W/m2 comes from the monthly
  // pump calculations.
  double n_E_pumps = 0.25; // Specific power of systems pumps + control systems in W/m2
  if (results.Qneed_cl > 0.0) {
    results.Qpump_tot = n_E_pumps * cooling->pumpControlReduction();
  } else if (results.Qneed_ht > 0.0) {
    results.Qpump_tot = n_E_pumps * heating->pumpControlReduction();
  } else {
    results.Qpump_tot = 0.0;
  }

  if (solarRadiationW > 0) { // Using W radiation because roof radiation isn't set up right now. 2015-02-18.
    results.Q_illum_ext_tot = 0; // No exterior lights during the day.
  } else {
    results.Q_illum_ext_tot = lights->exteriorEnergy() * exteriorLightingEnabled / structure->floorArea();
    //ExcelFunctions.printOut("CS156",exteriorLightingEnergyWperm2,0.0539503346043362);
  }

  results.Q_dhw = 0; //TODO no DHW calculations

  // Update tiHeatCool & TMT1 for next hour. tiHeatCool and TMT1 are passed by
  // reference to the function, allowing this information to pass from hour to
  // hour.
  double phiiHeatCool = phiActual + phii;
  // \Phi_{mtot} ISO 13790 C.3 eq. C.5
  double phimHeatCoolTotal = phimPhi0 + hem * temperature
      + h3 * (phisPhi0 + hwindowWperkm2 * temperature + h1 * (phiiHeatCool / hei + tEnteringAndSupplied)) / h2;
      // Set tmt to this hour's \theta_{m,t-1}.
  double tmt = TMT1;
  // \theta_{m,t}, ISO 13790 C.3 eq. C.4.
  // Set TMT1 to next hour's \theta_{m,t-1} (this hour's \theta_{m,t}).
  TMT1 = (TMT1 * (Cm / 3.6 - 0.5 * (h3 + hem)) + phimHeatCoolTotal) / (Cm / 3.6 + 0.5 * (h3 + hem));
  // \theta_{m}, ISO 13790 C.3 eq. C.9.
  double tmHeatCool = 0.5 * (TMT1 + tmt);
  // \theta_{s}, ISO 13790 C.3 eq. C.10.
  double tsHeatCool = (hms * tmHeatCool + phisPhi0 + hwindowWperkm2 * temperature + h1 * (tEnteringAndSupplied + phiiHeatCool / hei))
      / (hms + hwindowWperkm2 + h1);
  // \theta_{air}, ISO 13790, C.3 eq. C.11.
  tiHeatCool = (his * tsHeatCool + hei * tEnteringAndSupplied + phiiHeatCool) / (his + hei);

}


void ISOHourly::initialize()
{
  //TODO where do all these static numbers come from?
  fanDeltaPinPa = 800;
  fanN = 0.8;
  provisionalCFlowad = 1;
  solarPair = 0;
  intPair = 0.5;
  presenceSensorAd = 0.6;
  automaticAd = 0.8;
  presenceAutoAd = 0.6;
  manualSwitchAd = 1;
  presenceSensorLux = 500;
  automaticLux = 300;
  presenceAutoLux = 300;
  manualSwitchLux = 500;
  shadingRatioWtoM2 = 500;
  shadingMaximumUseRatio = 0.5;
  ventDcpWindImpact = 0.75;
  AtPerAFloor = 4.5;
  hci = 2.5;
  hri = 5.5;

  // TODO BAA@2014-12-22: This is still pretty rough and needs ought to be confirmed to be working correctly.
  auto lightingOccupancySensorDimmingFraction = building->lightingOccupancySensor();
  auto daylightSensorDimmingFraction = lights->dimmingFraction();

  if (lightingOccupancySensorDimmingFraction < 1.0 && daylightSensorDimmingFraction < 1.0) {
    maxRatioElectricLighting = presenceAutoAd;
    elightNatural = presenceAutoLux;
  } else if (lightingOccupancySensorDimmingFraction < 1.0) {
    maxRatioElectricLighting = presenceSensorAd;
    elightNatural = presenceSensorLux;
  } else if (daylightSensorDimmingFraction < 1.0) {
    maxRatioElectricLighting = automaticAd;
    elightNatural = automaticLux;
  } else {
    maxRatioElectricLighting = manualSwitchAd;
    elightNatural = manualSwitchLux;
  }

  double lightedNaturalAream2 = 0; // SingleBuilding.L53
  areaNaturallyLighted = std::max(0.0001, lightedNaturalAream2);
  areaNaturallyLightedRatio = areaNaturallyLighted / structure->floorArea();
  for (int i = 0; i < 9; i++) {
    this->structureCalculations(structure->windowShadingDevice(), structure->wallArea()[i], structure->windowArea()[i], structure->wallUniform()[i],
        structure->windowUniform()[i], structure->wallSolarAbsorbtion()[i], structure->windowShadingCorrectionFactor()[i],
        structure->windowNormalIncidenceSolarEnergyTransmittance()[i], i);
  }

  shadingUsePerWPerM2 = shadingMaximumUseRatio / shadingRatioWtoM2;
  nlaWMovableShadingH = nlams[ROOF] / structure->floorArea();
  naturalLightRatioH = nla[ROOF] / structure->floorArea();
  naturalLightShadeRatioReductionH = nlaWMovableShadingH - naturalLightRatioH;
  nlaWMovableShadingW = nlams[WEST] / structure->floorArea();
  naturalLightRatioW = nla[WEST] / structure->floorArea();
  naturalLightShadeRatioReductionW = nlaWMovableShadingW - naturalLightRatioW;
  nlaWMovableShadingS = nlams[SOUTH] / structure->floorArea();
  naturalLightRatioS = nla[SOUTH] / structure->floorArea();
  naturalLightShadeRatioReductionS = nlaWMovableShadingS - naturalLightRatioS;
  nlaWMovableShadingE = nlams[EAST] / structure->floorArea();
  naturalLightRatioE = nla[EAST] / structure->floorArea();
  naturalLightShadeRatioReductionE = nlaWMovableShadingE - naturalLightRatioE;
  nlaWMovableShadingN = nlams[NORTH] / structure->floorArea();
  naturalLightRatioN = nla[NORTH] / structure->floorArea();
  naturalLightShadeRatioReductionN = nlaWMovableShadingN - naturalLightRatioN;
  saWMovableShadingH = sams[ROOF] / structure->floorArea();
  solarRatioH = sa[ROOF] / structure->floorArea();
  solarShadeRatioReductionH = saWMovableShadingH - solarRatioH;
  saWMovableShadingW = sams[WEST] / structure->floorArea();
  solarRatioW = sa[WEST] / structure->floorArea();
  solarShadeRatioReductionW = saWMovableShadingW - solarRatioW;
  saWMovableShadingS = sams[SOUTH] / structure->floorArea();
  solarRatioS = sa[SOUTH] / structure->floorArea();
  solarShadeRatioReductionS = saWMovableShadingS - solarRatioS;
  saWMovableShadingE = sams[EAST] / structure->floorArea();
  solarRatioE = sa[EAST] / structure->floorArea();
  solarShadeRatioReductionE = saWMovableShadingE - solarRatioE;
  saWMovableShadingN = sams[NORTH] / structure->floorArea();
  solarRatioN = sa[NORTH] / structure->floorArea();
  solarShadeRatioReductionN = saWMovableShadingN - solarRatioN;

  // ISO 15242 Air leakage values.
  // Air leakage at 50 Pa in air-changes/hr. (Such as from blower door test).
  double n50 = 2; // SingleBldg.V4
  // Total air leakage at 4Pa in m3/hr. ISO 15242 Annex D Table D.1.
  double buildingv8 = 0.19 * (n50 * (structure->floorArea() * structure->buildingHeight()));
  // Air leakage per area at 4Pa (m3/hr/m2).
  q4Pa = std::max(0.000001, buildingv8 / structure->floorArea());

  P96 = hri * 1.2;
  // ISO 13790 12.2.2
  P97 = hci + P96; // TODO Does it make sense to do P97=hci+hri*1.2 and eliminate P96?
  // ISO 13790 7.2.2.2
  P98 = 1 / hci - 1 / P97; // (or 1/3.45).

  // ISO 13790 7.2.2.2 eq. 9 
  //
  // Eq in ISO is H_{tr,is} = h_{is} * A_{tot}
  // where A_{tot} = \Lambda_{at} * A_{f}
  //
  // P98 is 1/h_{is} (not sure why its done this way).
  // Eq here is H_{tr,is} = \Lambda_{at} / (1/h_{is})
  //            H_{tr,is} = \Lambda_{at} * h_{is}
  //
  // This is the "per floor area" version of eq. 9.
  his = AtPerAFloor / P98;

  // Calculate Cm from the data in the .ism file.
  // Units seem to need to be in KJ, so divide by 1000.
  double Cm_int = structure->interiorHeatCapacity() / 1000.0;
  // Convert env Cm to per floor area.
  double Cm_env = (structure->wallHeatCapacity() * sum(structure->wallArea()) / structure->floorArea()) / 1000.0;
  Cm = Cm_int + Cm_env;

  // Calculate Am based the Cm value and the default values in ISO 13790 12.3.1.2 Table 12.
  if (Cm > 370.0) {
    Am = 3.5;
  } else if (Cm > 260.0) {
    // Am = 3.0 @ Cm = 260; Am = 3.5 @ Cm = 370.
    Am = 3.0 + 0.5 * ((Cm - 260) / 110);
  } else if (Cm > 165.0) {
    // Am = 2.5 @ Cm = 165; Am = 3.0 @ Cm = 260.
    Am = 2.5 + 0.5 * ((Cm - 165) / 95);
  } else {
    Am = 2.5;
  }

  double hWind = 0, hWall = 0;
  for (int i = 0; i < ROOF + 1; i++) {
    hWind += hWindow[i];
    hWall += htot[i] - hWindow[i];
  }
  hwindowWperkm2 = hWind / structure->floorArea();

      // Constant portion of \Phi_{st}, i.e. without multiplying by
      // (.5*\Phi_{int} + \Phi_{sol}).  ISO 13790 C.2 eq. C.3.
  prs = (AtPerAFloor - Am - hwindowWperkm2 / P97) / AtPerAFloor;
  // intPair = 0.5, this ends up providing the ".5" in ".5*\Phi_{int}" in
  // eq. C.3. When used in phisPhi0.
  prsInterior = (1 - intPair) * prs;
  prsSolar = (1 - solarPair) * prs;

  // Constant portion of \Phi_{m}, i.e. without multiplying by
  // (.5*\Phi_{int} + \Phi_{sol}).  ISO 13790 C.2 eq. C.2.
  prm = Am / AtPerAFloor;
  prmInterior = (1 - intPair) * prm;
  prmSolar = (1 - solarPair) * prm;

  // ISO 13790 12.2.2 eq. 64
  hms = P97 * Am;

  hOpaqueWperkm2 = std::max(hWall / structure->floorArea(), 0.000001);

  // ISO 13790 12.2.2 eq. 63
  hem = 1 / (1 / hOpaqueWperkm2 - 1 / hms);

  double hzone = 39;
  windImpactHz = std::max(0.1, hzone);
  windImpactSupplyRatio = std::max(0.00001, ventilation->fanControlFactor()); //TODO ventSupplyExhaustRatio = SingleBuilding.P40 ?
}

void ISOHourly::populateSchedules()
{
  int dayStart = (int) pop->daysStart();
  int dayEnd = (int) pop->daysEnd();
  int hourStart = (int) pop->hoursStart();
  int hourEnd = (int) pop->hoursEnd();

  bool hoccupied, doccupied, popoccupied;
  for (int h = 0; h < 24; h++) {
    hoccupied = h >= hourStart && h < hourEnd;
    for (int d = 0; d < 7; d++) {
      doccupied = (d >= dayStart && d < dayEnd);
      popoccupied = hoccupied && doccupied;
      fixedVentilationSchedule[h][d] = hoccupied ? ventilation->supplyRate() : 0.0;
      fixedFanSchedule[h][d] = hoccupied ? 1 : 0.0;
      fixedExteriorEquipmentSchedule[h][d] = hoccupied ? 0.3 : 0.12;
      fixedInteriorEquipmentSchedule[h][d] = popoccupied ? 0.9 : 0.3;
      fixedExteriorLightingSchedule[h][d] = 1; // in calculateHour, the lights are only turned on when the sun is down.
      fixedInteriorLightingSchedule[h][d] = popoccupied ? 0.9 : 0.05;
      fixedActualHeatingSetpoint[h][d] = popoccupied ? heating->temperatureSetPointOccupied() : heating->temperatureSetPointUnoccupied();
      fixedActualCoolingSetpoint[h][d] = popoccupied ? cooling->temperatureSetPointOccupied() : cooling->temperatureSetPointUnoccupied();
    }
  }
}

// TODO BAA@2015-01-28 Is there a better place to keep these debug functions?
void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2)
{
  //if(DEBUG_ISO_MODEL_SIMULATION)
  {
    std::cout << matName << "(" << dim1 << ", " << dim2 << "): " << std::endl << "\t";
    for (unsigned int j = 0; j < dim2; j++) {
      std::cout << "," << j;
    }
    std::cout << std::endl;
    for (unsigned int i = 0; i < dim1; i++) {
      std::cout << "\t" << i;
      for (unsigned int j = 0; j < dim2; j++) {
        std::cout << "," << mat[i * dim2 + j];
      }
      std::cout << std::endl;
    }
  }
}

std::vector<double> ISOHourly::sumHoursByMonth(const std::vector<double>& hourlyData)
{
  std::vector<double> monthlyData(12);
  std::vector<int> monthsInHours = { 0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760 };

  for (int month = 0; month < 12; ++month) {
    monthlyData[month] = std::accumulate(hourlyData.begin() + monthsInHours[month],
                                         hourlyData.begin() + monthsInHours[month + 1],
                                         0.0);
  }

  return monthlyData;
}

const int ISOHourly::SOUTH = 0;
const int ISOHourly::SOUTHEAST = 1;
const int ISOHourly::EAST = 2;
const int ISOHourly::NORTHEAST = 3;
const int ISOHourly::NORTH = 4;
const int ISOHourly::NORTHWEST = 5;
const int ISOHourly::WEST = 6;
const int ISOHourly::SOUTHWEST = 7;
const int ISOHourly::ROOF = 8;

}
}
