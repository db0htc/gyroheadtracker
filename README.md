# Gyro Headtracker and mouse control device

  This project is a simple gyro-controlled headtracker and general mouse control tool, made for mounting on hats or anything else that follows head movement.
  It started as something i was making for a friend in Star Citizen who had issues with his hands, but he fell more ill and could not play as much.
  I'm putting it here for anyone else who wants a cool home-made headtracker or gyro assist device for FPS games. You can control your gun movements with it, since it uses mouse input. you can even control the gun gimbals in Star citizen while flying with sticks (very accurately, very quickly), but i do not recommend it. It was simpler to fly normally and use it as a head tracker

  Included in the repository are files for 3D printing the mount for the head tracker PCBs and buttons

  Bill of Materials:
  
Adafruit ESP32-S3 Feather with 4MB Flash 2MB PSRAM - STEMMA QT / Qwiic PID: 5477
https://www.adafruit.com/product/5477 

Adafruit 9-DOF Orientation IMU Fusion Breakout - BNO085 (BNO080) - STEMMA QT / Qwiic PID: 4754
https://www.adafruit.com/product/4754

STEMMA QT / Qwiic JST SH 4-pin Cable - 100mm Long
https://www.adafruit.com/product/4210 

at the time of this writing, the total cost before shipping is under $50. Not bad for a cheap head tracker with low latency and good accuracy!
Please support Adafruit by buying from them, their code libraries and hardware has made so many projects like this one possible, and
their contributions the arduino ecosystem can not be understated

This project should work with any ESP-32-S3 adjacent board, and any BNO080 gyro using I2C, but i recommend the three links above for plug and play simplicity

The above board is bluetooth capable, enabling a wireless version, but the bluetooth interface for mice is a bit weird and i didn't like it

due to the nature of geometry, the tracking follows a rotational path on a flat surface, so you will get more speed in the center of the screen, but slightly more accuracy and less speed on the edges of the screen. If you have a curved monitor, this is a non issue except on the vertical axis. You might not notice the effect when you edit the variables for mouse control vs head tracking

the following picture is how i made mine (STL file included)

![alt text](https://github.com/db0htc/gyroheadtracker/blob/main/TrackerMount.jpg "example of mounting")
https://github.com/db0htc/gyroheadtracker/blob/main/TrackerMount.jpg

No AI was used in the making of this project. Except to look up the normalization formula. That was hand-typed in. This project was simple enough not to need it, and i learned a lot more about software equivalents to hardware solutions (button de-bounce, toggles etc)
