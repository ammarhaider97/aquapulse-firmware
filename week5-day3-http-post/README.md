# Week 5 - Day 3: HTTP POST to Mock Flask Endpoint

## What this does
Sends the JSON payload (from Day 1) to a mock Flask server every 10s
via esp_http_client, but only when WiFi is connected (Day 2's event
group bit is checked first).

## Config needed
Replace MOCK_SERVER_URL with the actual endpoint provided by your source/internship before running.

## How to test in Wokwi
Watch serial monitor for "[HTTP] POST complete, status = 200" (or
whatever your mock server returns). If WiFi drops mid-run, watch it
correctly print "Skipped -- WiFi not connected yet" instead of crashing.

## Status
End-to-end pipe works for the happy path; fault injection + full task
integration comes Day 4.
