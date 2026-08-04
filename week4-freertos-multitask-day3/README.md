## Day 3: Watchdog

   Aggregator task is subscribed to the Task Watchdog Timer (5s timeout,
   panic-reboot on miss). Sensor tasks are not individually watchdog-guarded
   since the aggregator is the single point that would stall the whole
   pipeline.
