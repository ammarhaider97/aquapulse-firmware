# Week 1 - Day 2: GPIO Interrupt + ADC Read

Button-triggered LED toggle using GPIO interrupt, plus ADC1 channel reading
from a potentiometer (simulating a sensor input).
Simulated in Wokwi (ESP32-S3, ESP-IDF).

## What it does
- Configures a GPIO pin as input with edge-triggered interrupt
- ISR sets a flag on button press; main loop toggles LED based on flag
- Configures ADC1 channel (12-bit width, 11dB attenuation) connected to a potentiometer
- Reads raw ADC value and prints it to serial monitor once per second (1 Hz)

## Concepts covered
- GPIO interrupt (ISR) instead of polling
- volatile-qualified flag variable for ISR-to-main communication
- 12-bit SAR ADC read
- ADC attenuation for 0-3.3V input range

## Files
- main.c - firmware code
- diagram.json - Wokwi circuit wiring (button + potentiometer)

## Status
Base setup for Week 1 ADC precision work (oversampling + median filtering, Thursday/Friday).
