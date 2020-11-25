import isomodel

user_model = isomodel.UserModel()
user_model.load("./test_data/defaults_test_building.ism", "./test_data/defaults_test_defaults.ism")
monthly_model = user_model.toMonthlyModel()
results = monthly_model.simulate()
for result in results:
  print(result.getEndUse(isomodel.ElecCool))

print(isomodel.totalEnergyUse(results))

user_model = isomodel.UserModel()
user_model.load("./test_data/defaults_test_building.ism", "./test_data/defaults_test_defaults.ism")
hourly_model = user_model.toHourlyModel()
results = hourly_model.simulate()
for result in results:
  print(result.getEndUse(isomodel.ElecCool))

print(isomodel.totalEnergyUse(results))