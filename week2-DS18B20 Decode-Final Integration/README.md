## Day 4 — DS18B20 + Integration
- One-wire scratchpad decode (LSB/MSB -> °C, 0.0625°C/bit, handles negative temps)
- All modules integrated into a single `aquapulse_calibration` ESP-IDF component
- Full test suite runs at boot: pH, TDS, turbidity, EC validation, DS18B20

## Deliverable status
`aquapulse_calibration` component complete, all unit tests passing (synthetic ADC inputs).
