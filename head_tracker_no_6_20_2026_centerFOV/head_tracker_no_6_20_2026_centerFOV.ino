/*
This is a gyro-controlled mouse mover. It also works as a head tracker in some games (i use it for nuclear option and star citizen)
with the right code additions, this could also be used to create a unique joystick input or gamepad input

this used example code as an initial template, originally by Adafruit
who are awesome and deserve good things for how much they bring into the maker community
seriously, chinese products are cheaper but buy Adafruit if you can!

latency is at or less than 10ms (or was)
very accurate, can be tweaked by editing the two variables at the current default values for head tracking:
float FOV = 60;
float sensitivity = 1000;
these variables are found below at lines 64 and 65

an FOV value of 40 and a sensitivity of 500 is better for casual mouse control
FOV value of 40 was obtained by measuring my monitor and distance from my face to the monitor

sensitivity in part is left over from the time i had an absolute position version, 
but it's literally unusable in games due to them using relative positioning of the mouse. 
Might be better for accessibility users on the desktop, but requires additional files i have edited

the included libraries below are necessary for the project to compile

the two pins being used are the reset pin, and pin 18 (A0) for selecting the center of the monitor as the reference point. A0 (pin 18) is pulled low and activated by a button wired to positive
*/
#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include "Adafruit_TinyUSB.h"

#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS) || defined(ARDUINO_NRF52840_CIRCUITPLAY)
const int pin = 4; // Left Button
bool activeState = true;

#elif defined(ARDUINO_FUNHOUSE_ESP32S2)
const int pin = BUTTON_DOWN;
bool activeState = true;

#elif defined PIN_BUTTON1
const int pin = PIN_BUTTON1;
bool activeState = false;

#elif defined(ARDUINO_ARCH_ESP32)
const int pin = 0;
bool activeState = false;

#elif defined(ARDUINO_ARCH_RP2040)
const int pin = D0;
bool activeState = false;
#else
const int pin = A0;
bool activeState = false;
#endif


int16_t lastx = 0;
int16_t lasty = 0;

float rlimitx = 0;
float llimitx = 0;
float ulimity = 0;
float dlimity = 0;
float xcenter = 0;
float ycenter = 0;
float FOV = 60;
float sensitivity = 1000;

const int calbutton = 18;

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_MOUSE()
};

Adafruit_USBD_HID usb_hid;
#define BNO08X_CS 10
#define BNO08X_INT 9
#define FAST_MODE
#define BNO08X_RESET -1

struct euler_t {
  float yaw;
  float pitch;
  float roll;
} ypr;

Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

sh2_SensorId_t reportType = SH2_GYRO_INTEGRATED_RV;
long reportIntervalUs = 2000;

void setReports(sh2_SensorId_t reportType, long report_interval) {
  if (! bno08x.enableReport(reportType, report_interval)) {
  }
}

void setup(void) {
// for some reason the disconnect/reconnect was necessary on my computer
 digitalWrite(calbutton, HIGH);

 tud_disconnect();

 delay(2000);

 tud_connect();

 delay(2000);

 while (!TinyUSBDevice.isInitialized()) {
    delay(200);
    TinyUSBDevice.begin(0);
  }

 delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  if (!bno08x.begin_I2C()) {
    while (1) { delay(10); }
  }

  setReports(reportType, reportIntervalUs);

  pinMode(pin, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);

  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_MOUSE);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB Mouse");
  usb_hid.begin();

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  delay(100);
}

void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees = false) {

    float sqr = sq(qr);
    float sqi = sq(qi);
    float sqj = sq(qj);
    float sqk = sq(qk);

    ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
    ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
    ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

    if (degrees) {
      ypr->yaw *= RAD_TO_DEG;
      ypr->pitch *= RAD_TO_DEG;
      ypr->roll *= RAD_TO_DEG;
    }
}

void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

void quaternionToEulerGI(sh2_GyroIntegratedRV_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

void process_hid() {
  bool btn_pressed = digitalRead(calbutton);

  if (TinyUSBDevice.suspended()) {
    TinyUSBDevice.remoteWakeup();
  }

 if (btn_pressed) {
      delay(100);
        xcenter = ypr.yaw;
        ycenter = ypr.pitch;
        float FOVDIV = FOV / 2;
        ulimity = ycenter - FOVDIV;
        dlimity = ycenter + FOVDIV;
        llimitx = xcenter + FOVDIV;
        rlimitx = xcenter - FOVDIV;
 }
  return;
}

void loop() {

  #ifdef TINYUSB_NEED_POLLING_TASK
  TinyUSBDevice.task();
  #endif

  if (!TinyUSBDevice.mounted()) {
    return;
  }

  static uint32_t ms = 0;
  if (millis() - ms > 10) {
    ms = millis();
    process_hid();
  }

  if (bno08x.wasReset()) {
    setReports(reportType, reportIntervalUs);
  }
  
  if (bno08x.getSensorEvent(&sensorValue)) {
    // in this demo only one report type will be received depending on FAST_MODE define (above)
    switch (sensorValue.sensorId) {
      case SH2_ARVR_STABILIZED_RV:
        quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV, &ypr, true);
      case SH2_GYRO_INTEGRATED_RV:
        // faster (more noise?)
        quaternionToEulerGI(&sensorValue.un.gyroIntegratedRV, &ypr, true);
        break;
    }
    if (usb_hid.ready()) {
    uint8_t const report_id = 0; // no ID
    int16_t const x = (1 - ((ypr.yaw - rlimitx) / (llimitx - rlimitx))) * sensitivity;
    int16_t const y = ((ypr.pitch - ulimity) / (dlimity - ulimity)) * sensitivity;
    int8_t const relx = x - lastx;
    int8_t const rely = y - lasty;
    float const xyratio = (llimitx - rlimitx) / (dlimity - ulimity);
    lastx = x;
    lasty = y;
    usb_hid.mouseMove(report_id, relx, rely);
    /* //debug and development serial monitor printing
    Serial.print("limitset:"); Serial.print(limitset);  Serial.print("\n");
    Serial.print("rlimitx:");  Serial.print(rlimitx); Serial.print("\n");
    Serial.print("ypr.yaw:");  Serial.print(ypr.yaw);  Serial.print("\n");
    Serial.print("llimitx:");  Serial.print(llimitx);  Serial.print("\n");
    Serial.print("ulimity:");  Serial.print(ulimity);  Serial.print("\n");
    Serial.print("ypr.pitch:");  Serial.print(ypr.pitch);  Serial.print("\n");
    Serial.print("dlimity:");  Serial.print(dlimity);  Serial.print("\n"); 
    Serial.print("relx:");  Serial.print(relx);  Serial.print("\n");
    Serial.print("rely:");  Serial.print(rely);  Serial.print("\n");
    Serial.print("lastx:");  Serial.print(lastx);  Serial.print("\n");
    Serial.print("lasty:");  Serial.print(lasty);  Serial.print("\n");
    Serial.print("x:");  Serial.print(x);  Serial.print("\n");
    Serial.print("y:");  Serial.print(y);  Serial.print("\n");
    Serial.print("xyratio:");  Serial.print(xyratio);  Serial.print("\n");
    */
    //delay(100); // for easier serial monitoring
    //usb_hid.ABSmouseMove(report_id, x, y);
  }
  }

}
