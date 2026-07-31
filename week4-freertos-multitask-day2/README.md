## Day 2: Queues + Aggregator

   Single shared queue (tagged by sensor_id) carries readings from all 5
   sensor tasks to one aggregator task, which owns the OLED display and
   prints the combined snapshot to serial.
