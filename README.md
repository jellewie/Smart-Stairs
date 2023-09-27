# WIP
This project is still Work In Progress, things are bound to be changed

# Arduino-smart-stairs
A nice way to add some reactive lighting to your stairs. controlled by an ESP32 and a custom PCB, and powered by a 5V power supply.

You will need 0.3W watt per 100% bright white (RBG=255,255,255) LED. My stair for example needs maximum 13steps x 30LED x 0.3W = 114W, but I use a verry low brightness and not all steps are on at the same time. So a lower 100W power supply is used. If you unserspec the power supply the lights will change color to yellow-ish and weird things can happen.

# What you need
- [an PCB and all it parts](https://oshwlab.com/jellewietsma/smart-stairs)
- [Presure sensor for each step, like the SEN0299](https://eu.mouser.com/ProductDetail/DFRobot/SEN0299?qs=Zz7%252BYVVL6bEMMkhXlCdCeg%3D%3D)

# Appendix
* Firmware
[This is included in this repository](Arduino)
* PCB & schematic
[This is included in this repository](Schematic-PCB)
Beta/ updated version on [EasyEDA](https://oshwlab.com/jellewietsma/smart-stairs)
* 3D models
[These are included in this repository](3DModel)
