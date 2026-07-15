# Week 2 — AquaPulse Sensor Calibration Modules

   ESP-IDF component-style calibration logic (pH, TDS, turbidity, EC
   cross-validation, DS18B20) — currently kept inside a single `main.c`
   for simplicity, will be split into a proper `aquapulse_calibration`
   component once the structure is finalized.

   ## Day 1 — pH
   - Nernst equation (ideal) + two-point linear calibration
   - Unit tests run automatically at boot (`test_ph_run()`), results printed
     in Wokwi serial monitor.
