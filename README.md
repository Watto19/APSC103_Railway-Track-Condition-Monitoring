This is coded provided to FSME for sensing changes in the cross-sectional area of train tracks
To begin use:
- First ensure a 9V battery is hooked up to the Arduino
- Place the micro-SD into the slot at the back of the Arduino (located next to battery container)
- Flick the on switch mounnted to the case
- Let the device stay stationary allowing for time for the leveling code to calibrate
Now the device can be moved (best results at a constant speed) at speeds ranging from 0 - 3km/h

To collect the data from the SD card, remove the SD card from its container once the device has been
turned off, no place it into a micro-SD card reader and a excel file will appear. The distance is 
measured in inches and data has been converted to pitch and roll. As well, for ease of use data has 
been measured by 20ft increments the cart has passed (meaning if 35 ft travelled it will show as
1 and 15ft travelled, indicating you have travelled 15ft from the first track addition).
