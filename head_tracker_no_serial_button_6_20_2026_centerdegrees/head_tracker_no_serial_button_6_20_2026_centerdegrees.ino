#include <Arduino.h>
// This demo explores two reports (SH2_ARVR_STABILIZED_RV and SH2_GYRO_INTEGRATED_RV) both can be used to give 
// quartenion and euler (yaw, pitch roll) angles.  Toggle the FAST_MODE define to see other report.  
// Note sensorValue.status gives calibration accuracy (which improves over time)
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

bool waspressed = 0;

uint8_t limitset = 0;

bool uhoh = 0;

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_MOUSE()
};

// USB HID object
Adafruit_USBD_HID usb_hid;



// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9


#define FAST_MODE

// For SPI mode, we also need a RESET 
//#define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1

struct euler_t {
  float yaw;
  float pitch;
  float roll;
} ypr;

Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

#ifdef FAST_MODE
  // Top frequency is reported to be 1000Hz (but freq is somewhat variable)
  sh2_SensorId_t reportType = SH2_GYRO_INTEGRATED_RV;
  long reportIntervalUs = 2000;
#else
  // Top frequency is about 250Hz but this report is more accurate
  sh2_SensorId_t reportType = SH2_ARVR_STABILIZED_RV;
  long reportIntervalUs = 5000;
#endif
void setReports(sh2_SensorId_t reportType, long report_interval) {
//  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(reportType, report_interval)) {
//    Serial.println("Could not enable stabilized remote vector");
  }
}

void setup(void) {


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



//  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
  //if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte UART buffer!
  //if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
  //  Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
 // Serial.println("BNO08x Found!");


  setReports(reportType, reportIntervalUs);

 // Serial.println("Reading events");


  pinMode(pin, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);

  // Set up HID
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_MOUSE);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB Mouse");
  usb_hid.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

 // Serial.println("Adafruit TinyUSB HID Mouse example");

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
  // Whether button is pressed
  bool btn_pressed = digitalRead(calbutton);

  // nothing to do if button is not pressed


  // Remote wakeup
  if (TinyUSBDevice.suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
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
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // poll gpio once each 10 ms
  static uint32_t ms = 0;
  if (millis() - ms > 10) {
    ms = millis();
    process_hid();
  }

  if (bno08x.wasReset()) {
   // Serial.print("sensor was reset ");
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
    if (usb_hid.ready() && !uhoh) {
    uint8_t const report_id = 0; // no ID
    int16_t const x = (1 - ((ypr.yaw - rlimitx) / (llimitx - rlimitx))) * sensitivity;
    int16_t const y = ((ypr.pitch - ulimity) / (dlimity - ulimity)) * sensitivity;
    int8_t const relx = x - lastx;
    int8_t const rely = y - lasty;
    float const xyratio = (llimitx - rlimitx) / (dlimity - ulimity);
    lastx = x;
    lasty = y;
    usb_hid.mouseMove(report_id, relx, rely);
    /*
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
