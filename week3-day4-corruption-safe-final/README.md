# Day 4 — Corruption-Safe NVS + Factory Reset (Week 3 Final)
Adds magic-number + checksum validation on top of Day 3's NVS storage.
On corruption or missing data, automatically falls back to factory defaults.
Button: short press = save a test calibration update; long press (>2s) =
esp_restart() to simulate a reboot and verify persistence.
