# Week 7 — Integrated Firmware Simulation & Handoff

Final integration week for the AquaPulse firmware: combines all prior
modules (sensor tasks, calibration, NVS, OLED, RTOS scheduler, JSON/WiFi
pipeline, watchdog) into a single tested firmware, plus a light-sleep
power management demo.

## What was added this week
- Light sleep demo (`esp_sleep_enable_timer_wakeup` + `esp_light_sleep_start`)
  via a dedicated low-priority task, proving the power management pattern
  without disrupting sensor timing
- Watchdog registration extended from just the aggregator task to all
  five sensor tasks
- 60-minute continuous soak test (see `soak-test-log.txt`)
- NVS reboot persistence re-verified after integration (see `nvs-reboot-test.md`)
- Live JSON delivery to a mock endpoint (webhook.site) verified end-to-end
- Fault injection test verified: out-of-range values are rejected and
  flagged, never crash the JSON pipeline (see `json-fault-oled-test.md`)
- OLED display confirmed in sync with the values sent over the network

## Architecture

\`\`\`mermaid
flowchart TD
    subgraph Sensors["Sensor Tasks (FreeRTOS, own interval + watchdog)"]
        PH[ph_task - 2s]
        TDS[tds_task - 2s]
        TUR[turbidity_task - 5s]
        EC[ec_task - 5s]
        TMP[temp_task - 1s]
    end

    CAL[(Calibration data\nin RAM, mutex-protected)]
    NVS[(NVS Flash\ncalibration persistence)]

    PH --> Q[Sensor Data Queue]
    TDS --> Q
    TUR --> Q
    EC --> Q
    TMP --> Q

    CAL -.read.-> PH
    CAL -.read.-> TMP
    CAL <-.save/load.-> NVS

    Q --> AGG[Aggregator Task\nwatchdog-protected]
    AGG --> SNAP[(g_readings snapshot\nmutex-protected)]
    AGG --> OLED[SSD1306 OLED\nlive display]

    SNAP --> NET[Network Task\nevery 10s]
    NET --> VAL{Range check\nper reading}
    VAL -->|valid| JSON[Build JSON payload]
    VAL -->|invalid| FLAG[Set fault_flags\nzero out bad field]
    FLAG --> JSON
    JSON --> WIFI[WiFi STA\nauto-reconnect + backoff]
    WIFI --> HTTP[HTTP POST\nmock endpoint]

    PWR[Power Mgmt Task\nlight sleep every 5s] -.pauses whole chip\nbriefly, low priority.-> Sensors
\`\`\`

## How to run
1. Open the Wokwi project (link below)
2. Click "Start simulation"
3. Watch the serial monitor for `[AGG]`, `[NET]`, `[PWR]`, and `[UPTIME]` logs
4. OLED updates live with pH / TDS / turbidity / EC / temperature

## Test evidence
- `soak-test-log.txt` — 60-minute continuous run, no watchdog reset
- `nvs-reboot-test.md` — calibration survives simulated reboot
- `json-fault-oled-test.md` — valid JSON, fault flag rejection, OLED sync
