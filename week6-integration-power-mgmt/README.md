# Week 7 - Day 1: Power Management Integration

Added:
- Light sleep demo (esp_sleep_enable_timer_wakeup + esp_light_sleep_start)
  via a dedicated low-priority power_mgmt_task, 200ms sleep every 5s cycle
- Watchdog registration + reset extended to all 5 sensor tasks
  (previously only the aggregator task was watchdog-protected)

Note: light sleep pauses the whole chip, so the sleep window is kept
short to avoid disturbing sensor polling intervals or triggering the
watchdog. See main README for full architecture notes.
