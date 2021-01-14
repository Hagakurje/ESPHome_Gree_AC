#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Gree.h"

const uint16_t kIrLed = 14;
IRGreeAC ac(kIrLed);

class GreeAC : public Component, public Climate
{
public:
  void setup() override
  {
    ac.begin();
    ac.on();

    ac.setMode(kGreeAuto);
    ac.setTemp(22);
    ac.setFan(kGreeFanAuto);
    ac.setSwingVertical(true, kGreeSwingAuto);
  }

  climate::ClimateTraits traits()
  {
    auto traits = climate::ClimateTraits();

    traits.set_supports_current_temperature(true);
    traits.set_supports_two_point_target_temperature(false);
    traits.set_visual_min_temperature(16);
    traits.set_visual_max_temperature(30);
    traits.set_visual_temperature_step(1.f);

    traits.set_supports_auto_mode(true);
    traits.set_supports_cool_mode(true);
    traits.set_supports_heat_mode(true);
    traits.set_supports_fan_only_mode(false);
    traits.set_supports_dry_mode(false);

    traits.set_supports_fan_mode_auto(true);
    traits.set_supports_fan_mode_low(true);
    traits.set_supports_fan_mode_medium(true);
    traits.set_supports_fan_mode_high(true);

    traits.set_supports_swing_mode_off(true);
    traits.set_supports_swing_mode_both(false);
    traits.set_supports_swing_mode_vertical(true);
    traits.set_supports_swing_mode_horizontal(false);

    return traits;
  }

  void control(const ClimateCall &call) override
  {
    if (call.get_mode().has_value())
    {
      ClimateMode climateMode = *call.get_mode();
      switch (climateMode)
      {
      case CLIMATE_MODE_HEAT:
        ac.setMode(kGreeHeat);
        ac.on();
        break;
      case CLIMATE_MODE_COOL:
        ac.setMode(kGreeCool);
        ac.on();
        break;
      case CLIMATE_MODE_AUTO:
        ac.setMode(kGreeAuto);
        ac.on();
        break;
      case CLIMATE_MODE_OFF:
        ac.off();
        break;
      }

      this->mode = climateMode;
      this->publish_state();
    }

    if (call.get_target_temperature().has_value())
    {
      float temp = *call.get_target_temperature();
      ac.setTemp(temp);

      this->target_temperature = temp;
      this->publish_state();
    }

    if (call.get_fan_mode().has_value())
    {
      ClimateFanMode fanMode = *call.get_fan_mode();
      switch (fanMode)
      {
      case CLIMATE_FAN_AUTO:
        ac.setFan(kGreeFanAuto);
        break;
      case CLIMATE_FAN_LOW:
        ac.setFan(kGreeFanMin);
        break;
      case CLIMATE_FAN_MEDIUM:
        ac.setFan(kGreeFanMed);
        break;
      case CLIMATE_FAN_HIGH:
        ac.setFan(kGreeFanMax);
        break;
      }

      this->fan_mode = fanMode;
      this->publish_state();
    }

    if (call.get_swing_mode().has_value())
    {
      ClimateSwingMode swingMode = *call.get_swing_mode();
      switch (swingMode)
      {
      case CLIMATE_SWING_OFF:
        ac.setSwingVertical(false, kGreeSwingMiddle);
        break;
      case CLIMATE_SWING_VERTICAL:
        ac.setSwingVertical(true, kGreeSwingAuto);
        break;
      }

      this->swing_mode = swingMode;
      this->publish_state();
    }

    ac.send();
    delay(200);
  }
};

class GreeApi : public Component, public CustomAPIDevice
{
public:
  void setup() override
  {
    register_service(&GreeApi::set_light, "set_light", {"light"});
  }

  void set_light(bool light)
  {
    ESP_LOGD("GreeApi", "Set light to ", light ? "ON" : "OFF");
    
    ac.setLight(light);
    ac.send();
    delay(200);
  }
};

class GreeSensor : public PollingComponent, public BinarySensor
{
public:
  GreeSensor() : PollingComponent(5000) {}

  void update() override
  {
    publish_state(ac.getLight());
  }
};