# Week 5 - Day 2: WiFi STA + Exponential Backoff Reconnect

## What this does
Connects ESP32 to Wokwi's simulated WiFi (Wokwi-GUEST). On disconnect,
retries with exponential backoff (1s -> 2s -> 4s ... capped at 60s),
resetting to 1s after a successful reconnect.

## How to test in Wokwi
Watch serial monitor for "[WiFi] Connected, IP: ...". To test backoff,
pause the Wokwi simulation mid-run and resume -- watch retry delay double
each failed attempt in the serial log.

## Status
WiFi connects; JSON payload from Day 1 still just prints to serial
(not sent anywhere yet).
