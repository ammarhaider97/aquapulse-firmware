# Week 5 - Day 4: Full Integration + Fault Injection Test

## What this does
Final week deliverable: a single `network_task` runs every 10s, reads
the latest sensor snapshot, validates + builds the JSON payload
(rejecting/flagging out-of-range fields), and POSTs it to the mock
Flask endpoint if WiFi is connected. Includes a fault-injection test
mode (TEST_FAULT_INJECTION) that forces an out-of-range pH every 3rd
cycle to prove rejection works end-to-end.

## Deliverable checklist
- Valid JSON POSTed every 10s: YES
- Fault injection test demonstrates rejection: YES (see serial log,
  fault_flags bit 0 set on injected cycles)
- Reconnect on-loss with exponential backoff: YES (Day 2)
- BLE: explicitly deferred to hardware phase, NimBLE covered via
  ESP-IDF docs review only -- no code this week

## Config needed before running
Replace MOCK_SERVER_URL with the real endpoint.
