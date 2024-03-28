# Arduino-smart-stairs
A nice way to add some reactive lighting to your stairs. controlled by an ESP32 and a custom PCB, and powered by a 5V power supply.
<img align="right" src="Images/Home Assistant dashboard.png" alt="Home Assistant dashboard example" width=30%>
This also can be intergraded to Home [Assistant with](https://www.home-assistant.io/) [MQTT](https://www.home-assistant.io/integrations/mqtt)

You will need 0.3W watt per 100% bright white (RBG=255,255,255) LED. My stair for example needs maximum 13steps x 30LED x 0.3W = 114W, but I use a verry low brightness and not all steps are on at the same time. So a lower 100W power supply is used. If you unserspec the power supply the lights will change colour to yellow-ish and weird things can happen.

### OTA (Over The Air update)
This page can be accessed on [smart-stairs.local/ota](http://smart-stairs.local/ota) (or 'IP/ota') and enables you to update firmware over WiFi.
On this page is a 'choose file' button where the new version can be selected. Make sure the right, compatible, most updated file is selected ("smart-stairs.bin"). This can be downloaded from [releases](https://github.com/jellewie/Smart-Stairs/releases). 
After which the 'Upload' button needs to be pressed for the update process to begin, the unit will automatically update and reboot afterward.
Note that [SoftSettings](#soft-settings) are preserved.

### Full reset
If a full reset is desired it can be achieved by going to 'smart-stairs.local/reset'. But note that accessing the page will directly wipe all [SoftSettings](#soft-settings) from existence and there will be no way to restore them back. If the wipe was successful, it will be reported back and will execute a restart.

### Soft settings
There are multiple soft settings, these are settings that can be changed after the sketch has been uploaded, but are still saved so they can be restored after a reboot.
The most up-to-date values can be found in the top of the [WiFiManagerBefore.h](Arduino/WiFiManagerBefore.h) source file, and can only be set in [smart-stairs.local/ip](http://smart-stairs.local/ip).
These settings are saved directly after APmode and in the [smart-stairs.local/ip](http://smart-stairs.local/ip) page.
Note that the character " and TAB (EEPROM_Seperator) cannot be used, these will be replaced with ' and SPACE respectively. Leave black to skip updating these, use spaces ' ' to clear the values
- **name** The name of the device (reboot required when changing)
- **HABrokerIP** The IP of you MQTT broker, probably just your Home Assistant
- **HABrokerUser** The user which to use to log into the MQTT broker
- **HABrokerPass** The corresponding password for the user
- **LDRmax** the maximum (byte 0-255) of after which the stair will keep off. This prevents the stair from being on during the day. Put at 255 to be always on (Can be changed from Home Assistant). Note the current value can also be set in HA

#### Pages
- **OTA** will direct to the [OTA](#ota-over-the-air-update) update page, where the firmware can be updated over the WiFi.
- **Reset** will fully [restart](#full-reset) the ESP.
- **Info** [/info](http://smart-stairs.local/info) some debug info like the version compile date.
- **Saved settings** [/ip](http://smart-stairs.local/ip) to show you the values saved in the EEPROM.

# What you need
- [an PCB and all it parts](https://oshwlab.com/jellewietsma/smart-stairs)
- [Pressure sensor for each step, like the SEN0299](https://eu.mouser.com/ProductDetail/DFRobot/SEN0299?qs=Zz7%252BYVVL6bEMMkhXlCdCeg%3D%3D)

# Appendix
* Firmware
[This is included in this repository](Arduino)
* PCB & schematic
[This is included in this repository](Schematic-PCB)
Beta/ updated version on [EasyEDA](https://oshwlab.com/jellewietsma/smart-stairs)
* 3D models
[These are included in this repository](3DModel)
