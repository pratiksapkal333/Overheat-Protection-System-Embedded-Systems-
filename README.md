# Overheat-Protection-System-Embedded-Systems-

- Engineered an automated thermal management system using an ESP8266 microcontroller and DS18B20 digital sensors for high-precision temperature monitoring.
- Developed firmware in C++ (Arduino IDE) to sample data at 30-second intervals and display real-time readings on an integrated LED display.
- Implemented a dual-stage safety logic:
    -Alert Stage: Configured a GSM module to transmit emergency SMS alerts when reaching the first temperature threshold.
    -Cut-off Stage: Designed a hardware interlock using Relays to automatically trigger a system shutdown at critical temperatures to prevent hardware failure.
- Optimized power and logic flow to ensure 24/7 reliability of the monitoring circuit
