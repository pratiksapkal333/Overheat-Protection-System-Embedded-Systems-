#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pin configuration
const int rs = D1, en = D2, d4 = D3, d5 = D6, d6 = D7, d7 = D8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// DS18B20 setup
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// GSM setup - using hardware serial
#define GSM_SERIAL Serial

// Relay setup
const int relayPin = D0;

// Phone number for alerts
const String phoneNumber = "+917987332242";

// Temperature thresholds
const float WARNING_TEMP = 40.0;
const float CRITICAL_TEMP = 70.0;

void setup() {
    // Initialize serial for debugging
    Serial.begin(9600);
    delay(5000);
    Serial.println("\nSystem Initializing...");

    // Initialize GSM serial
    GSM_SERIAL.begin(9600);

    // Initialize LCD
    lcd.begin(16, 2);
    Serial.println("LCD Initialized");

    // Initialize temperature sensor
    sensors.begin();
    Serial.println("Temperature Sensor Initialized");

    // Initialize relay
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH); // Start with relay off
    Serial.println("Relay Initialized (OFF)");

    // Display welcome message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Starting");
    lcd.setCursor(0, 1);
    lcd.print("Checking Sensors...");
    Serial.println("Displaying welcome message");

    delay(10000);

    // Check all sensors
    bool allOK = true;
    static bool smsSent = false;
    // Check temperature sensor
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    if (temp == DEVICE_DISCONNECTED_C) {
        lcd.clear();
        lcd.print("Temp Sensor Fail");
        Serial.println("ERROR: Temperature sensor disconnected");
        allOK = false;
        delay(2000);
    }
    else {
        Serial.print("Temperature sensor OK. Initial temp: ");
        Serial.print(temp);
        Serial.println("°C");
    }
    lcd.clear();
    String iMsg = "Motor On ";
    sendSMS(iMsg);
    smsSent = true;
    // Update LCD
    lcd.setCursor(0, 1);
    lcd.print("Motor on- SMS sent");
    delay(2000);
    // Check GSM module 
    if (allOK) {
        lcd.clear();
        lcd.print("All Systems OK");
        Serial.println("All systems check OK");
        delay(1000);
    }
}

void loop() {
    static unsigned long lastTempTime = 0;
    static bool smsSent = false;
    static bool relayTriggered = false;

    // Get temperature every 30 seconds
    if (millis() - lastTempTime >= 10000) {
        lastTempTime = millis();

        sensors.requestTemperatures();
        float tempC = sensors.getTempCByIndex(0);

        // Display temperature on LCD and Serial
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temperature:");
        lcd.setCursor(0, 1);
        lcd.print(tempC);
        lcd.print((char)223); // Degree symbol
        lcd.print("C");

        Serial.print("Current Temperature: ");
        Serial.print(tempC);
        Serial.println("°C");

        // Check temperature thresholds
        if (tempC > CRITICAL_TEMP && !relayTriggered) {
            // Send critical alert SMS
            String alertMsg = "CRITICAL ALERT: Temp. reached " + String(tempC) + "C. Motor OFF.";
            sendSMS(alertMsg);
            Serial.println("Sent critical alert SMS");

            // Critical temperature - shut off relay
            digitalWrite(relayPin, LOW);
            relayTriggered = true;
            Serial.println("CRITICAL TEMPERATURE! Relay turned OFF");

            // Update LCD
            lcd.clear();
            lcd.print("CRITICAL TEMP!");
            lcd.setCursor(0, 1);
            lcd.print("System Shutdown");
            delay(5000);
        }
        else if (tempC > WARNING_TEMP && !smsSent && !relayTriggered) {
            // Warning temperature - send alert
            String warningMsg = "Warning: Temp reached " + String(tempC) + "C";
            sendSMS(warningMsg);
            smsSent = true;
            Serial.println("Sent warning SMS");

            // Update LCD
            lcd.setCursor(0, 1);
            lcd.print("WARNING SENT!");
            delay(2000);
        }
        else if (tempC <= WARNING_TEMP) {
            // Temperature back to normal
            if (smsSent) {
                Serial.println("Temperature returned to normal range");
                smsSent = false;
            }

            // If relay was triggered but temp is now safe
            if (relayTriggered && tempC < WARNING_TEMP - 5) {
                digitalWrite(relayPin, HIGH);
                relayTriggered = false;
                String normalMsg = "Temperature back to normal: " + String(tempC) + "C. Motor restarted.";
                sendSMS(normalMsg);
                Serial.println("System restarted after cooldown");
            }
        }
    }

    // Small delay to prevent watchdog reset
    delay(100);
}

void sendSMS(String message) {
    Serial.print("Attempting to send SMS: ");
    Serial.println(message);

    // Set GSM to text mode
    GSM_SERIAL.println("AT+CMGF=1");
    delay(1000);

    // Set recipient number
    GSM_SERIAL.print("AT+CMGS=\"");
    GSM_SERIAL.print(phoneNumber);
    GSM_SERIAL.println("\"");
    delay(1000);

    // Send message
    GSM_SERIAL.print(message);
    delay(1000);

    // Send Ctrl+Z to end message
    GSM_SERIAL.write(26);
    delay(1000);

    // Read response
    if (GSM_SERIAL.available()) {
        String response = GSM_SERIAL.readString();
        Serial.print("SMS Response: ");
        Serial.println(response);
    }
}