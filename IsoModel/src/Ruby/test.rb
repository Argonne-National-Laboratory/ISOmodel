require 'test/unit'
require 'isomodel'

class IsoModelTest < Test::Unit::TestCase

  def test_defaults_test_building
    user_model = IsoModel::UserModel.new
    user_model.load("./test_data/defaults_test_building.ism", "./test_data/defaults_test_defaults.ism")
    assert_in_delta(0.9, user_model.terrainClass())

    monthly_model = user_model.toMonthlyModel
    results = monthly_model.simulate
    results.each do |result|
      puts result.getEndUse(IsoModel::ElecCool)
    end
    puts IsoModel::totalEnergyUse(results)
  end
end
