# Week 4 - Day 1: FreeRTOS Task Basics

   5 independent FreeRTOS tasks created, one per sensor, each running at
   its configured interval (pH/TDS: 2s, turbidity/EC: 5s, temperature: 1s)
   using vTaskDelayUntil for accurate periodic timing. Calibration struct
   is now protected by a mutex since multiple tasks read it.

   No queue yet — each task just prints its own reading. Aggregator comes
   in Day 2.
