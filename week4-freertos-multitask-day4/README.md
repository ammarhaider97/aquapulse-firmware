## Day 4: Stack Monitoring

   A dedicated low-priority task prints each task's stack high-water mark
   every 10s and flags any task with less than 20% of its allocated stack
   still free. All 5 sensor tasks + aggregator run concurrently, verified
   at their configured intervals in the serial log.
