# Active heater controller
The original controller of my heater is too buggy:
- loses state after a power loss;
- target temperature must be a round number;
- its thermocouple is not precise enough.
So an Arduino based controller is about to replace it. 

## Target features:
- [X] support all three speeds of the fan;
- [X] measure room temperature;
- [ ] measure heating water temperature;
- [ ] stop the fan if heating water is colder than target temperature;
- [X] display data on a segmented LCD;
- [X] settable target temperature via buttons;
- [ ] programmable schedule.

## Hardware used
Currently it works with a TMP36 sensor which should be replaced with a DHT22 for better accuracy and wider measuring bandwidth. The LCD used will be a 16x2 with an I2C controller. In a future version the controller should be split in two parts - one that controls the heater and a remote one with display, buttons and temperature sensor.
