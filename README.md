# mqtt-ir
A simple microcontroller firmware for controlling existing IR devices via MQTT (e.g. through Home Assistant).

## Preamble
This project provides you with a simple **IoT interface** for devices controlled via a **remote control**. It is meant to bridge the gap between your Smart Home infrastructure and your legacy devices in a **non-invasive** way by building a kind of seperate internet-connected remote control.

## Requirements
### Hardware
A dedicated device must be put together to be used with this piece of software. The following are key requirements for your circuit:
- A development board supported by the **Arduino SDK** (recommendation: **WEMOS LOLIN D1 Mini** or other board incorporating an **ESP8266** microcontroller)
- An **infrared light emitting diode** operating at the same frequency as your remote control (typically **940nm**) connected to one of the GPIO pins (recommendation: **Pin 4**)

<img src="https://user-images.githubusercontent.com/40141286/195071200-ec2e29dc-29ab-4be8-bb1d-87e1ddb6cffd.png" width=400px height=400px>

### Compilation
The following Arduino SDK libraries must be included for a successful compilation:
- crankyoldgit/IRremoteESP8266
- knolleary/PubSubClient

### Infrastructure
This project is meant to be incorporated into an IoT network. The following components are required for this element to function:
- An MQTT Broker (recommendation: [Eclipse Mosquitto](https://mosquitto.org/))
- An MQTT client and a front-end user interface (recommendation: [Home Assistant](https://www.home-assistant.io/))

## Configuration
### config.h
The **config.h** file must be edited to configure the following:
- Board model
- WiFi credentals
- Static IP address
- MQTT credentials
- MQTT base topic

### main.cpp
**NOTE:** The requirement for the **main.cpp** file to be manually edited is temporary. A future update will mitigate this need via migrating the configuration into a separate file.

The **main.cpp** file must be edited to configure the following:
- Amount of remote control "buttons" to be virtualized
- The [protocol](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md) used
- The [codes](https://github.com/crankyoldgit/IRremoteESP8266/tree/master/examples/IRrecvDumpV3) to be associated with given MQTT topic (button)

### Home Assistant
This section will focus on configuring Homeassistant, should that be your chosen front-end.
The configured MQTT topics must be added to the configuration file as follows:
```yaml
mqtt:
  button:
    - unique_id: bedroom_ceilinglamp_on
      name: "Ceiling Lamp Power"
      command_topic: "/bedroom/ceilinglamp/on"
      availability_topic: "/bedroom/ceilinglamp/available"
    - unique_id: bedroom_ceilinglamp_lamp
      name: "Cealing Lamp Toggle"
      command_topic: "/bedroom/ceilinglamp/lamp"
      availability_topic: "/bedroom/ceilinglamp/available"
    - unique_id: bedroom_ceilinglamp_cycle
      name: "Ceiling Lamp Cycle"
      command_topic: "/bedroom/ceilinglamp/cycle"
      availability_topic: "/bedroom/ceilinglamp/available"
    - unique_id: bedroom_ceilinglamp_red
      name: "Ceiling Lamp Red"
      command_topic: "/bedroom/ceilinglamp/red"
      availability_topic: "/bedroom/ceilinglamp/available"
    - unique_id: bedroom_ceilinglamp_green
      name: "Ceiling Lamp Green"
      command_topic: "/bedroom/ceilinglamp/green"
      availability_topic: "/bedroom/ceilinglamp/available"
    - unique_id: bedroom_ceilinglamp_blue
      name: "Ceiling Lamp Blue"
      command_topic: "/bedroom/ceilinglamp/blue"
      availability_topic: "/bedroom/ceilinglamp/available"
```
The entities created this way can simply be added to your dashboard.

Automatic discovery will be added in a future update.
