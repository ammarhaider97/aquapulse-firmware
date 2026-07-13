# Week 1 - Day 3 (Thursday): ADC Oversampling + Median Filter

Implements 16x oversampling and 5-sample median filter for clean ADC readings.

## What it does
- Reads ADC 16 times, averages for oversampling
- Nosie added manually as potentiometer is stable in wokwoi
- Repeats 5 times, takes median for final filtered value
- Prints raw vs filtered side-by-side
