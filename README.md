# Arduino-smart-stairs
A nice way to add some reactive lighting to your stairs. controlled by an ESP32 and a custom PCB, and powered by a 5V power supply.
<img align="right" src="Images/Home Assistant dashboard.png" alt="Home Assistant dashboard example" width=30%>
This also can be intergraded to [Home Assistant](https://www.home-assistant.io/) with [MQTT](https://www.home-assistant.io/integrations/mqtt)

You will need 0.3W watt per 100% bright white (RBG=255,255,255) LED. My stair for example needs maximum 13steps x 30LED x 0.3W = 114W, but I use a verry low brightness and not all steps are on at the same time. So a lower 100W power supply is used. If you unserspec the power supply the lights will change colour to yellow-ish and weird things can happen.

the light.smart_stairs_01_led_strip will have an 'effect' called Smart-Stairs which in which mode the stair animation is enabled. When the light is off the stair animation will not play. 

### settings
Controls:
- **LDRmax** the maximum (byte 0-4095) of after which the stair will keep off. This prevents the stair from being on during the day. Put at 4096 to be always on
- **ExtraDirection** Howmany steps extra there should be on in walking direction
- **LEDTimeIdle** The time 
- **LEDTimeOn** The time in ms that would set the minimum on-time of the step
- **TriggerThreshold** Minimum sensor value the step sensor needs to have before it counts it as pressed
Sensors:
- **LUX** The amount of raw light the sensor is currently measureing
- **Stair Active** If there is (or was recently) an user on the stairs
- **Direction Up** True if an user is walking upwards, false if they are walking down

# What you need
- [an PCB and all it parts](https://oshwlab.com/jellewietsma/smart-stairs)
- [Pressure sensor for each step, like the SEN0299](https://eu.mouser.com/ProductDetail/DFRobot/SEN0299?qs=Zz7%252BYVVL6bEMMkhXlCdCeg%3D%3D)

# Power consumption
If I only connect the 5V power supply it consumes about 2.1W,
If I connect the ESP32 to it it consumes 2.1W
If I also connect the LED's it uses 3.1W
These measurements are made with an athom smart-plug-v2 with an cse7766, so takes these numbers with a big grain of salt.

Total Stair power comsumption idle 3.1W

Esp (+PSU + sensors) 2.1W

PSU only 1.6W
# Appendix
* Firmware
[This is included in this repository](Arduino)
* PCB & schematic
[This is included in this repository](Schematic-PCB)
Beta/ updated version on [EasyEDA](https://oshwlab.com/jellewietsma/smart-stairs)
