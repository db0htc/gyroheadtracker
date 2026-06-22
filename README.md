# Gyro Headtracker and mouse control device

  This project is a simple gyro-controlled headtracker and general mouse control tool, made for mounting on hats or anything else that follows head movement
  It started as something i was making for a friend in Star Citizen who had issues with his hands, but he fell more ill and could not play as much
  I'm putting it here for anyone else who wants a cool home-made headtracker or gyro assist device for FPS games. You can control your gun movements with it, since it uses mouse input

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
