# Week 5 - Day 1: JSON Payload Builder + Fault Validation

## What this does
Builds the AquaPulse backend JSON payload from live sensor readings using
cJSON. Validates each field against a physically-plausible range before
including it; out-of-range fields are replaced with 0.0 and flagged in
`fault_flags` (bitmask).

## Payload schema
{ device_id, firmware_version, timestamp, readings: { ph, tds_ppm,
turbidity_ntu, temp_c, ec_us_cm }, fault_flags }

## How to test in Wokwi
Run the project, watch serial monitor -- a JSON string prints every 10s.
Every 3rd print injects a fake out-of-range pH value to prove the
validator rejects it (fault_flags bit 0 set, ph = 0.0 in output).

## Status
No WiFi/HTTP yet -- pure JSON generation + validation only.
