# Control Gree AC using ESPHome and IRremoteESP8266 - EXAMPLE

**IR remote** - Avatto S06, Geeklink IR Bridge

* Create file "gree_ir.h" in folder "config\esphome\gree", where "config" is HA configuration folder.
* Set remote transmitter pin **const uint16_t kIrLed = 14;**
* Edit your ".yaml" file like "ir_bedroom.yaml".

**UART:**
* port 3v3 of the device to the 3v3 of the converter.
* port TXD of the device to the RXD of the converter.
* port RXD of the device to the TXD of the converter.
* port GND of the device to the GND of the converter.
* port IO0 of the device to the GND of the converter.

**Flash** using ESPHome-Flasher https://github.com/esphome/esphome-flasher/releases. First flash with UART, other flashes on air.

**Result:**

![Bedroom AC](images/bedroom_ac.png)


**Current temperature sensor** - your current Home Assistant temperature sensor:

```
sensor:
  - platform: homeassistant
    id: current_temperature
    entity_id: sensor.temperature
```


**Set temperature sensor** - call **set_temp_sensor** method:

```
climate:
- platform: custom
  lambda: |-
    auto bedroom_ac = new GreeAC();
    bedroom_ac->set_temp_sensor(id(current_temperature)); 
    App.register_component(bedroom_ac);
    return {bedroom_ac};

  climates:
    - name: "Bedroom AC"
```


**Service esphome.ir_bedroom_set_data** - set all data to AC with one 'beep':

```
- service: esphome.ir_bedroom_set_data
  data:
    hvac: 'cool'
    temp: 22
    fan: 'auto'
    swing: 'off'
    light: True
```


**Switch switch.bedroom_ac_light** - current AC light switch.

### Remember and sync states after node restart

Add to HA configuration an `input_number` to store the target temperature:
```
input_number:
  status_climate_bedroom_temp:
    min: 0
    max: 50
```

Add to HA configuration a few `input_text` to store operation mode, fan mode, swing mode:

```
input_text:
  status_climate_bedroom_hvac:
  status_climate_bedroom_fan:
  status_climate_bedroom_swing:
```

Use automations to sync climate states with the statuses:

```
automation:
- id: sync_climate_bedroom_hvac
  alias: Sync Climate Bedroom HVAC mode
  initial_state: 'on'
  trigger:
    - platform: state
      entity_id: climate.bedroom_ac
  condition:
    condition: not
    conditions:
      - condition: state
        entity_id: climate.bedroom_ac
        state: "unavailable"
  action:
    - service: input_text.set_value
      target:
        entity_id: input_text.status_climate_bedroom_hvac
      data:
        value: "{{ states('climate.bedroom_ac') }}"

- id: sync_climate_bedroom_fan
  alias: Sync Climate Bedroom Fan
  trigger:
    - platform: state
      entity_id: climate.bedroom_ac
      attribute: fan_mode
  condition:
    condition: not
    conditions:
      - condition: state
        entity_id: climate.bedroom_ac
        attribute: fan_mode
        state: 'on'
  action:
    - service: input_text.set_value
      target:
        entity_id: input_text.status_climate_bedroom_fan
      data:
        value: "{{ state_attr('climate.bedroom_ac','fan_mode') }}"

- id: sync_climate_bedroom_swing
  alias: Sync Climate Bedroom Swing
  trigger:
    - platform: state
      entity_id: climate.bedroom_ac
      attribute: swing_mode
  action:
    - service: input_text.set_value
      target:
        entity_id: input_text.status_climate_bedroom_swing
      data:
        value: "{{ state_attr('climate.bedroom_ac','swing_mode') }}"

- id: sync_climate_bedroom_temp
  alias: Sync Climate Bedroom Temp
  trigger:
    - platform: state
      entity_id: climate.bedroom_ac
      attribute: temperature
  condition:
    condition: not
    conditions:
      - condition: state
        entity_id: climate.bedroom
        attribute: temperature
        state: 0
  action:
    - service: input_number.set_value
      target:
        entity_id: input_number.status_climate_bedroom_temp
      data:
        value: "{{ state_attr('climate.bedroom_ac','temperature') }}"



- id: sync_climate_bedroom_irblaster
  alias: Sync Climate Bedroom back to IR Blaster
  initial_state: 'on'
  trigger:
    platform: state
    entity_id: climate.bedroom_ac
    from: 'unavailable'
  action:
    - service: esphome.ir_bedroom_set_data
      data:
        temp: "{{ states('input_number.status_climate_bedroom_temp') }}"
        hvac: "{{ states('input_text.status_climate_bedroom_hvac') }}"
        fan: "{{ states('input_text.status_climate_bedroom_fan') }}"
        swing: "{{ states('input_text.status_climate_bedroom_swing') }}"
        light: "{{ false if is_state('input_text.status_climate_bedroom_hvac', 'off') else true }}"
```
