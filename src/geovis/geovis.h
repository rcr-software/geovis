#ifndef RCR_GEOVIS_GEOVIS_H_
#define RCR_GEOVIS_GEOVIS_H_

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// RCR headers
#include "atmospheric-sensor.h"
#include "constants.h"
#include "debug-settings.h"
#include "gps-receiver.h"
#include "inertial-measurement-unit.h"
#include "logger.h"
#include "novelty-printouts.h"

namespace rcr {
namespace geovis {

Logger logger;
AtmosphericSensor atmospheric_sensor; // Barometer/Thermometer/Hygometer
GpsReceiver gps_receiver;             // GPS module
InertialMeasurementUnit imu;          // IMU


inline void blink() {
  // Illuminate LED.
  digitalWrite(kLedPin, HIGH);
  
  // Ensure we are always receiving gps data.
  gps_receiver.smartDelay(kLoopDelay);
  while (Serial1.available() > 0) {
    gps_receiver.gps_.encode(Serial1.read());
  }

  // Dim LED.
  digitalWrite(kLedPin, LOW);
}

inline void initialize_objects() {
  // GPS receiver
  if (!gps_receiver.Init()) {
#if DEBUG_ANY
    Serial.println("GPS initialization failed.");
#endif
  }

  // IMU
  if (!imu.Init()) {
#if DEBUG_ANY
    Serial.println("IMU initialization failed.");
#endif
  }

  // Barometer/Thermometer/Hygometer
  if (!atmospheric_sensor.Init()) {
#if DEBUG_ANY
    Serial.println("Atmospheric sensor initialization failed.");
#endif
  }

  // Data logger
  if (!logger.Init()) {
#if DEBUG_ANY
    Serial.println("SD card initialization failed.");
#endif
  }
}

inline void setup() {
  // Wait a moment.
  delay(1024);

  // Set LED pin.
  pinMode(kLedPin, OUTPUT);

  // Start serial communication.
  Serial.begin(9600); // baud does not matter for Teensy 3.6
  Serial.println("In setup.");

  // Initialize objects.
  initialize_objects();
  Serial.println("init setup.");

  // Initialize output file(s).
  {
    // Set run-time resolved filename.
    String path = "";
    path += String{ millis() };
    path += ".csv";
    logger.Open(path);
  }
  {
    // Initialize with header for each comma-delimited value.
    String csv_header = "";
    csv_header += "millis,";                     // first column is time
    csv_header += atmospheric_sensor.kCsvHeader; // Atmospheric data header
    csv_header += gps_receiver.kCsvHeader;       // GPS header
    csv_header += imu.kCsvHeader;                // IMU header
    csv_header += "\n";

    // Write it only once.
    logger.Write(csv_header);
  }

  Serial.println("Setup complete.");
}

inline void fly() {
  static String csv_line = ""; // Set empty each time.
  static auto count = 0;

  // forever // TODO: terminate via radio instruction
  for (;;) {
    // Get a line of data.
    // - Time
    csv_line += millis();
    csv_line += ",";
#if DEBUG_ANY
    Serial.println(csv_line);
#endif

    // - Weather
    csv_line += atmospheric_sensor.GetCsvLine();
#if DEBUG_ATMOSPHERICSENSOR
    Serial.println(atmospheric_sensor.GetCsvLine());
#endif


    // - GPS
    csv_line += gps_receiver.GetCsvLine();
#if DEBUG_GPSRECEIVER
    Serial.println(gps_receiver.GetCsvLine());
#endif

    // - IMU
    csv_line += imu.GetCsvLine();
#if DEBUG_IMU
    Serial.println(csv_line/*imu.GetCsvLine()*/);
#endif


    // Print it to the file.
    csv_line += "\n";
    logger.Write(csv_line);

    // After a few writes, flush the file buffer. (To ensure data is written.)
    //auto count_after_five_seconds = 1000 / kLoopDelay * 5;
    if (++count > 128) {
      count = 0;
#if DEBUG_LOGGER
      Serial.println("XXXXXXXXXXXXX FLUSHING XXXXXXXXXXXXXXX");
#endif
      logger.Close();
      logger.Open(String{ millis() });
    }

    // Wait a moment.
    blink();
  }
}

bool menu_has_displayed = false;

// Allow calibration because our IMU's calibration persists only for one power
// cycle (has no on-board EEPROM).
inline void loop() {
  // Show the menu.
  if (!menu_has_displayed) {
    print_menu();
    menu_has_displayed = true;
  }

  // User decides.
  if (Serial.available() > 0) {
    switch (Serial.read()) {
      case 'c': {
        Serial.println("Received input [c].");
        print_with_ellipses("Beginning IMU calibration");
        imu.Calibrate();
        menu_has_displayed = false;
        break;
      }
      case 'f': {
        Serial.println("Received input [f].");
        print_with_ellipses("Beginning flight");
        fly();
        menu_has_displayed = false;
        break;
      }
      default: {
        print_with_ellipses("Input not recognized");
        break;
      }
    }

    // Clear remaining characters.
    while (Serial.available() > 0) {
      Serial.read();
    }
    Serial.flush();
    Serial.clear();
  }

  // Wait a moment.
  blink();
}

} // namespace geovis
} // namespace rcr

#endif // RCR_GEOVIS_GEOVIS_H_
