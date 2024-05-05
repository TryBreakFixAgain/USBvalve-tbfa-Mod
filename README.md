### *USBvalve TrafficLight and LiPo Mod*

Based on https://github.com/cecio/USBvalve Version 0.18.0

### *For basic information, please visit the cecio repository.*

### *Mods:*
- TrafficLight LED
- simple language customisation
- Fixing the first 2 lines
- LiPo Mod
  
As I often have to deal with people who are a bit impatient and forget their glasses from time to time, I added a traffic light LED to USBvalve.
To be independent of a power source, the LIPO mod based on a Pimoroni Pico Lipo followed.

I decided against a custom PCB and used standard components.

*All required parts can be found in the corresponding BOM.*

### *Mappings NO LiPo:*
- PIN6 of Pi --> OLED SDA
- PIN7 of Pi --> OLED SCL
- PIN19 of Pi --> D+ of USB Host
- PIN20 of Pi --> D- of USB Host
- PIN23 (GND) of Pi --> GND of USB Host
- PIN38 (GND) of Pi --> OLED GND
- PIN36 (3V3OUT) of Pi --> OLED VCC
- PIN40 (VBUS) of Pi --> VCC of USB Host
  
TrafficLight
I use 3.3V to power the LED because 5V was too bright for me.
Depending on the LED you may have to use 5V.

- PIN36 (3V3OUT) of Pi --> OLED VCC OR PIN40 (VBUS) of Pi --> VCC of Ws2812b DOT
- PIN28 (GND) of Pi --> GND of Ws2812b DOT
- PIN10 of Pi --> DIN of Ws2812b DOT
 
Additional BOOTSEL Button form y Case Design
- TP6 of Pi (Bottom side Testpoint) --> Pushbutton
- PIN28 (GND) of Pi --> Pushbutton


### *Mappings LiPo (Promoroni Pico LiPo):*
- PIN6 of Pi --> OLED SDA
- PIN7 of Pi --> OLED SCL
- PIN19 of Pi --> D+ of USB Host
- PIN20 of Pi --> D- of USB Host
- PIN38 (GND) of Pi --> OLED GND
- PIN36 (3V3OUT) of Pi --> OLED VCC
- PIN39 (VSYS) of Pi --> + IN DC-DC  3.7V to 5V Step-Up
- PIN23 (GND) of Pi --> - IN DC-DC  3.7V to 5V Step-Up
- \+ OUT DC-DC  3.7V to 5V Step-Up --> VCC of USB Host
- \- OUT DC-DC  3.7V to 5V Step-Up --> GND of USB Host

  
TrafficLight
I use 3.3V to power the LED because 5V was too bright for me.
Depending on the LED you may have to use 5V.

- PIN36 (3V3OUT) of Pi --> OLED VCC OR PIN40 (VBUS) of Pi --> VCC of Ws2812b DOT
- PIN28 (GND) of Pi --> GND of Ws2812b DOT
- PIN10 of Pi --> DIN of Ws2812b DOT

### *LED meanings:*
- Red: Things are happening that must not happen.
- Green: Everything is OK
- Orange: Attention, something could be wrong.
- Off: Nothing connected or nothing recognised. Attention: The LED remains off with some RubberDuckies. So if a stick is connected and the LED shows nothing, it could be a RubberDucky.


## SAFETY WARNING

> [!WARNING]
> I've received a lot of questions about **USBvalve** and *USB killer devices*. **USBvalve** is not built to test these devices, it has not any kind of insulation or protection, so if you have the suspect you are dealing with one of these devices, test it with something else, NOT with **USBvalve** or you may damage the device, yourself or objects near to you.
