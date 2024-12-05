#include <IRremote.hpp>

/**
 * Smart Outlet Control with IR Remote and Sound Detection
 * @Author: George Morales
 */

// Pin Definitions
const int SOUND_SENSOR = A0;    // LM393 Sound Sensor connected to analog pin A0
const int RELAY_SIG = 4;        // Relay power pin
const int IR_RECEIVE_PIN = 5;   // IR receiver pin

// State Variables
boolean OUTLET_STATUS = false;
int CALIBRATION_TIME = 5000;
boolean SOUND_ACTIVATED = false;
int clap_threshold = 100;  // Initial threshold (to be calibrated)

void setup() {
    Serial.begin(9600);
    
    // Configure pins
    pinMode(RELAY_SIG, OUTPUT);
   // pinMode(SOUND_SENSOR, INPUT);
    
    // Initialize infrared sensor module
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    Serial.println("IR Receiver Ready");
}


void loop() {
    // Read sound sensor
    int sound_sensor_output = analogRead(SOUND_SENSOR);
    Serial.println(sound_sensor_output);

    // Calibrate sound sensor whenever its first switched on
    if (sound_sensor_output > 20 && SOUND_ACTIVATED == false) {
      calibrateSoundSensor();
      SOUND_ACTIVATED = true;
    }
    
    // Sound-based control
    if (sound_sensor_output > clap_threshold) {
        toggleOutlet();
        delay(100); // Debounce delay
    }

    // IR-based control
    if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.command == 0x40) { // 0x40 corresponds to the 'power' button
            toggleOutlet();
            delay(100); // Debounce delay
        }
        
        //Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.resume();
    }
    
    delay(100); // Prevent serial buffer overflow
}


/**
 * Toggles the outlet state
 */
void toggleOutlet() {
    OUTLET_STATUS = !OUTLET_STATUS;
    if (OUTLET_STATUS == true) {
      digitalWrite(RELAY_SIG, LOW);
    } else {
      digitalWrite(RELAY_SIG, HIGH);
    }
}


/**
 * Calibrates the sound sensor threshold
 * Runs for 5 seconds to establish baseline threshold of analog values
 */
void calibrateSoundSensor() {
    long sum = 0;
    int num_readings = 0;
    unsigned long start_time = millis();
    
    Serial.println("Calibrating sound sensor...");
    
    while (millis() - start_time < CALIBRATION_TIME) {
        sum += analogRead(SOUND_SENSOR);
        num_readings++;
        delay(10);
    }
    
    int average = sum / num_readings;
    clap_threshold = average + 15;
  
    Serial.println("Calibration complete. Thresholds set to: ");
    Serial.println(clap_threshold);    
}
