ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce3808,len:0x370
load:0x403c9700,len:0x900
load:0x403cc700,len:0x2364
entry 0x403c98ac
[WiFi] STA init done, connecting to "Wokwi-GUEST"...
[0;31mE (357) task_wdt: esp_task_wdt_init(515): TWDT already initialized[0m
TWDT already initialized by IDF defaults -- continuing
Startup: FACTORY DEFAULTS
[AGG] updated pH = 7.20 (tick=32)
[AGG] updated TDS = 819.16 (tick=32)
[AGG] updated NTU = 1000.00 (tick=32)
[AGG] updated EC = 2506.23 (tick=32)
[AGG] updated TMP = 24.50 (tick=32)
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":0.33,"readings":{"ph":7.1973819732666016,"tds_ppm":819.157470703125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":0}
[AGG] pH=7.20 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=132)
[AGG] pH=7.20 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 7.20 (tick=230)
[AGG] pH=7.20 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=233)
[AGG] updated TMP = 24.50 (tick=233)
[AGG] pH=7.20 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[WiFi] Connected, IP: 10.10.0.2
[AGG] updated TMP = 24.50 (tick=332)
[AGG] pH=7.20 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 7.20 (tick=430)
[AGG] pH=7.20 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=433)
[AGG] updated TMP = 24.50 (tick=433)
[AGG] pH=7.20 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
Short press -> calibration updated & saved
[PWR] Entering light sleep for 200 ms
[AGG] updated NTU = 1000.00 (tick=533)
[PWR] Woke up from light sleep
[AGG] pH=7.20 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=536)
[AGG] updated EC = 2506.23 (tick=537)
[AGG] pH=7.20 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[0;31mE (5607) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:[0m
[0;31mE (5607) task_wdt:  - network_task (CPU 0/1)[0m
[0;31mE (5607) task_wdt: Tasks currently running:[0m
[0;31mE (5607) task_wdt: CPU 0: IDLE0[0m
[0;31mE (5607) task_wdt: CPU 1: IDLE1[0m
[0;31mE (5607) task_wdt: Print CPU 0 (current core) backtrace[0m


Backtrace: 0x4200E66F:0x3FC9EB30 0x4200EA8C:0x3FC9EB50 0x40377C89:0x3FC9EB80 0x4037BCDF:0x3FCA9850 0x420033CA:0x3FCA9870 0x40380B19:0x3FCA9890 0x4037F8F5:0x3FCA98B0

[0;31mE (5607) task_wdt: Print CPU 1 backtrace[0m


Backtrace: 0x40379356:0x3FC9F160 0x40377C89:0x3FC9F180 0x4037BCDF:0x3FCA9FB0 0x420033CA:0x3FCA9FD0 0x40380B19:0x3FCA9FF0 0x4037F8F5:0x3FCAA010

[AGG] updated pH = 7.56 (tick=630)
[AGG] pH=7.56 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=633)
[AGG] updated TMP = 24.50 (tick=633)
[AGG] pH=7.56 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=732)
[AGG] pH=7.56 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 7.56 (tick=830)
[AGG] pH=7.56 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=833)
[AGG] updated TMP = 24.50 (tick=833)
[AGG] pH=7.56 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=932)
[AGG] pH=7.56 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[AGG] updated pH = 12.76 (tick=1034)
[PWR] Woke up from light sleep
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":10.35,"readings":{"ph":12.759952545166016,"tds_ppm":827.4317626953125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":0}
---- Stack high-water mark report ----
  ph_task          free=1984 bytes / 3072 bytes (64.6% free)
[AGG] pH=12.76 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
  tds_task         free=1992 bytes / 3072 bytes (64.8% free)
[AGG] updated TMP = 24.50 (tick=1038)
[AGG] updated EC = 2506.23 (tick=1039)
[AGG] updated NTU = 1000.00 (tick=1039)
[AGG] updated TDS = 827.43 (tick=1039)
  turbidity_task   free=1496 bytes / 2560 bytes (58.4% free)
  ec_task          free=1480 bytes / 2560 bytes (57.8% free)
  temp_task        free=1144 bytes / 2048 bytes (55.9% free)
  aggregator_task  free=2088 bytes / 4096 bytes (51.0% free)
---------------------------------------
[AGG] pH=12.76 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1132)
[AGG] pH=12.76 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
Short press -> calibration updated & saved
[AGG] updated pH = 13.37 (tick=1230)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1233)
[AGG] updated TMP = 24.50 (tick=1233)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1332)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 13.37 (tick=1430)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1433)
[AGG] updated TMP = 24.50 (tick=1433)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[PWR] Woke up from light sleep
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1539)
[AGG] updated EC = 2506.23 (tick=1539)
[AGG] updated NTU = 1000.00 (tick=1539)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[0;31mE (15627) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:[0m
[0;31mE (15627) task_wdt:  - network_task (CPU 0/1)[0m
[0;31mE (15627) task_wdt: Tasks currently running:[0m
[0;31mE (15627) task_wdt: CPU 0: IDLE0[0m
[0;31mE (15627) task_wdt: CPU 1: IDLE1[0m
[0;31mE (15627) task_wdt: Print CPU 0 (current core) backtrace[0m


Backtrace: 0x4200E66F:0x3FC9EB30 0x4200EA8C:0x3FC9EB50 0x40377C89:0x3FC9EB80 0x4037BCDF:0x3FCA9850 0x420033CA:0x3FCA9870 0x40380B19:0x3FCA9890 0x4037F8F5:0x3FCA98B0

[0;31mE (15627) task_wdt: Print CPU 1 backtrace[0m


Backtrace: 0x40379356:0x3FC9F160 0x40377C89:0x3FC9F180 0x4037BCDF:0x3FCA9FB0 0x420033CA:0x3FCA9FD0 0x40380B19:0x3FCA9FF0 0x4037F8F5:0x3FCAA010

[AGG] updated pH = 13.37 (tick=1630)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1633)
[AGG] updated TMP = 24.50 (tick=1633)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1732)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 13.37 (tick=1830)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1833)
[AGG] updated TMP = 24.50 (tick=1833)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1932)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
Long press -> esp_restart()
[WiFi] Disconnected. Reconnecting in 1000 ms (backoff)
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xc (RTC_SW_CPU_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce3808,len:0x370
load:0x403c9700,len:0x900
load:0x403cc700,len:0x2364
entry 0x403c98ac
[WiFi] STA init done, connecting to "Wokwi-GUEST"...
[0;31mE (19405) task_wdt: esp_task_wdt_init(515): TWDT already initialized[0m
TWDT already initialized by IDF defaults -- continuing
Startup: LOADED FROM NVS
[AGG] updated pH = 13.37 (tick=28)
[AGG] updated TDS = 819.16 (tick=28)
[AGG] updated NTU = 1000.00 (tick=28)
[AGG] updated EC = 2506.23 (tick=28)
[AGG] updated TMP = 24.50 (tick=29)
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":0.29,"readings":{"ph":13.367568969726562,"tds_ppm":819.157470703125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":0}
[AGG] pH=13.37 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=128)
[AGG] pH=13.37 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[WiFi] Connected, IP: 10.10.0.2
[AGG] updated pH = 13.37 (tick=226)
[AGG] pH=13.37 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=229)
[AGG] updated TMP = 24.50 (tick=229)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=328)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 13.37 (tick=426)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=429)
[AGG] updated TMP = 24.50 (tick=429)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[AGG] updated TMP = 24.50 (tick=531)
[PWR] Woke up from light sleep
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated NTU = 1000.00 (tick=534)
[AGG] updated EC = 2506.23 (tick=534)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[0;31mE (24655) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:[0m
[0;31mE (24655) task_wdt:  - network_task (CPU 0/1)[0m
[0;31mE (24655) task_wdt: Tasks currently running:[0m
[0;31mE (24655) task_wdt: CPU 0: IDLE0[0m
[0;31mE (24655) task_wdt: CPU 1: IDLE1[0m
[0;31mE (24655) task_wdt: Print CPU 0 (current core) backtrace[0m


Backtrace: 0x4200E66F:0x3FC9EB30 0x4200EA8C:0x3FC9EB50 0x40377C89:0x3FC9EB80 0x4037BCDF:0x3FCA9850 0x420033CA:0x3FCA9870 0x40380B19:0x3FCA9890 0x4037F8F5:0x3FCA98B0

[0;31mE (24655) task_wdt: Print CPU 1 backtrace[0m


Backtrace: 0x40379356:0x3FC9F160 0x40377C89:0x3FC9F180 0x4037BCDF:0x3FCA9FB0 0x420033CA:0x3FCA9FD0 0x40380B19:0x3FCA9FF0 0x4037F8F5:0x3FCAA010

[AGG] updated pH = 13.37 (tick=626)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=629)
[AGG] updated TMP = 24.50 (tick=629)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=728)
[AGG] pH=13.37 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
Short press -> calibration updated & saved
[AGG] updated pH = 13.98 (tick=826)
[AGG] pH=13.98 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=829)
[AGG] updated TMP = 24.50 (tick=829)
[AGG] pH=13.98 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=928)
[AGG] pH=13.98 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
Short press -> calibration updated & saved
[PWR] Entering light sleep for 200 ms
[PWR] Woke up from light sleep
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":10.31,"readings":{"ph":13.975184440612793,"tds_ppm":827.4317626953125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":0}
---- Stack high-water mark report ----
  ph_task          free=1984 bytes / 3072 bytes (64.6% free)
[AGG] pH=13.98 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1034)
[AGG] updated pH = 14.58 (tick=1035)
[AGG] updated EC = 2506.23 (tick=1035)
[AGG] updated NTU = 1000.00 (tick=1035)
[AGG] updated TDS = 827.43 (tick=1036)
  tds_task         free=1980 bytes / 3072 bytes (64.5% free)
  turbidity_task   free=1472 bytes / 2560 bytes (57.5% free)
  ec_task          free=1476 bytes / 2560 bytes (57.7% free)
  temp_task        free=1144 bytes / 2048 bytes (55.9% free)
  aggregator_task  free=2092 bytes / 4096 bytes (51.1% free)
---------------------------------------
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1128)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=1226)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1229)
[AGG] updated TMP = 24.50 (tick=1229)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1328)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=1426)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1429)
[AGG] updated TMP = 24.50 (tick=1429)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[PWR] Woke up from light sleep
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1536)
[AGG] updated NTU = 1000.00 (tick=1536)
[AGG] updated EC = 2506.23 (tick=1536)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[0;31mE (34665) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:[0m
[0;31mE (34665) task_wdt:  - network_task (CPU 0/1)[0m
[0;31mE (34665) task_wdt: Tasks currently running:[0m
[0;31mE (34665) task_wdt: CPU 0: IDLE0[0m
[0;31mE (34665) task_wdt: CPU 1: IDLE1[0m
[0;31mE (34665) task_wdt: Print CPU 0 (current core) backtrace[0m


Backtrace: 0x4200E66F:0x3FC9EB30 0x4200EA8C:0x3FC9EB50 0x40377C89:0x3FC9EB80 0x4037BCDF:0x3FCA9850 0x420033CA:0x3FCA9870 0x40380B19:0x3FCA9890 0x4037F8F5:0x3FCA98B0

[0;31mE (34665) task_wdt: Print CPU 1 backtrace[0m


Backtrace: 0x40379356:0x3FC9F160 0x40377C89:0x3FC9F180 0x4037BCDF:0x3FCA9FB0 0x420033CA:0x3FCA9FD0 0x40380B19:0x3FCA9FF0 0x4037F8F5:0x3FCAA010

Long press -> esp_restart()
[WiFi] Disconnected. Reconnecting in 1000 ms (backoff)
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xc (RTC_SW_CPU_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce3808,len:0x370
load:0x403c9700,len:0x900
load:0x403cc700,len:0x2364
entry 0x403c98ac
[WiFi] STA init done, connecting to "Wokwi-GUEST"...
[0;31mE (7725) task_wdt: esp_task_wdt_init(515): TWDT already initialized[0m
TWDT already initialized by IDF defaults -- continuing
Startup: LOADED FROM NVS
[AGG] updated pH = 14.58 (tick=28)
[AGG] updated TDS = 819.16 (tick=28)
[AGG] updated NTU = 1000.00 (tick=28)
[AGG] updated EC = 2506.23 (tick=28)
[AGG] updated TMP = 24.50 (tick=28)
[NET] fault_flags = 0x01 -- rejected field(s) present
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":0.29,"readings":{"ph":0,"tds_ppm":819.157470703125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":1}
[AGG] pH=14.58 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=128)
[AGG] pH=14.58 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[WiFi] Connected, IP: 10.10.0.2
[AGG] updated pH = 14.58 (tick=226)
[AGG] pH=14.58 TDS=819 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=229)
[AGG] updated TMP = 24.50 (tick=229)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=328)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=426)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=429)
[AGG] updated TMP = 24.50 (tick=429)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[AGG] updated TMP = 24.50 (tick=529)
[PWR] Woke up from light sleep
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated EC = 2506.23 (tick=532)
[AGG] updated NTU = 1000.00 (tick=532)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[0;31mE (12965) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:[0m
[0;31mE (12965) task_wdt:  - network_task (CPU 0/1)[0m
[0;31mE (12965) task_wdt: Tasks currently running:[0m
[0;31mE (12965) task_wdt: CPU 0: IDLE0[0m
[0;31mE (12965) task_wdt: CPU 1: IDLE1[0m
[0;31mE (12965) task_wdt: Print CPU 0 (current core) backtrace[0m


Backtrace: 0x4200E66F:0x3FC9EB30 0x4200EA8C:0x3FC9EB50 0x40377C89:0x3FC9EB80 0x4037BCDF:0x3FCA9850 0x420033CA:0x3FCA9870 0x40380B19:0x3FCA9890 0x4037F8F5:0x3FCA98B0

[0;31mE (12965) task_wdt: Print CPU 1 backtrace[0m


Backtrace: 0x40379356:0x3FC9F160 0x40377C89:0x3FC9F180 0x4037BCDF:0x3FCA9FB0 0x420033CA:0x3FCA9FD0 0x40380B19:0x3FCA9FF0 0x4037F8F5:0x3FCAA010

[AGG] updated pH = 14.58 (tick=626)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=629)
[AGG] updated TMP = 24.50 (tick=629)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=728)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=826)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=829)
[AGG] updated TMP = 24.50 (tick=829)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=928)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[AGG] updated TMP = 24.50 (tick=1029)
[PWR] Woke up from light sleep
[NET] fault_flags = 0x01 -- rejected field(s) present
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":10.29,"readings":{"ph":0,"tds_ppm":827.4317626953125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":1}
---- Stack high-water mark report ----
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1032)
[AGG] updated NTU = 1000.00 (tick=1032)
[AGG] updated EC = 2506.23 (tick=1033)
[AGG] updated pH = 14.58 (tick=1033)
  ph_task          free=1984 bytes / 3072 bytes (64.6% free)
  tds_task         free=1980 bytes / 3072 bytes (64.5% free)
  turbidity_task   free=1472 bytes / 2560 bytes (57.5% free)
  ec_task          free=1460 bytes / 2560 bytes (57.0% free)
  temp_task        free=1144 bytes / 2048 bytes (55.9% free)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
  aggregator_task  free=2092 bytes / 4096 bytes (51.1% free)
---------------------------------------
[AGG] updated TMP = 24.50 (tick=1128)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1226)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=1229)
[AGG] updated TMP = 24.50 (tick=1229)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1328)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1426)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=1429)
[AGG] updated TMP = 24.50 (tick=1429)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[AGG] updated NTU = 1000.00 (tick=1531)
[PWR] Woke up from light sleep
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1535)
[AGG] updated EC = 2506.23 (tick=1535)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[0;31mE (22965) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:[0m
[0;31mE (22965) task_wdt:  - network_task (CPU 0/1)[0m
[0;31mE (22965) task_wdt: Tasks currently running:[0m
[0;31mE (22965) task_wdt: CPU 0: IDLE0[0m
[0;31mE (22965) task_wdt: CPU 1: IDLE1[0m
[0;31mE (22965) task_wdt: Print CPU 0 (current core) backtrace[0m


Backtrace: 0x4200E66F:0x3FC9EB30 0x4200EA8C:0x3FC9EB50 0x40377C89:0x3FC9EB80 0x4037BCDF:0x3FCA9850 0x420033CA:0x3FCA9870 0x40380B19:0x3FCA9890 0x4037F8F5:0x3FCA98B0

[0;31mE (22965) task_wdt: Print CPU 1 backtrace[0m


Backtrace: 0x40379356:0x3FC9F160 0x40377C89:0x3FC9F180 0x4037BCDF:0x3FCA9FB0 0x420033CA:0x3FCA9FD0 0x40380B19:0x3FCA9FF0 0x4037F8F5:0x3FCAA010

[AGG] updated TDS = 827.43 (tick=1626)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=1629)
[AGG] updated TMP = 24.50 (tick=1629)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1728)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TDS = 827.43 (tick=1826)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated pH = 14.58 (tick=1829)
[AGG] updated TMP = 24.50 (tick=1829)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=1928)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[PWR] Entering light sleep for 200 ms
[PWR] Woke up from light sleep
[TEST] Fault injection: forcing pH = 20.0 (out of range)
[NET] fault_flags = 0x01 -- rejected field(s) present
[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: {"device_id":"AQUAPULSE-001","firmware_version":"1.5.0","timestamp":20.32,"readings":{"ph":0,"tds_ppm":827.4317626953125,"turbidity_ntu":1000,"temp_c":24.5,"ec_us_cm":2506.22705078125},"fault_flags":1}
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
[AGG] updated TMP = 24.50 (tick=2035)
[AGG] updated TDS = 827.43 (tick=2035)
[AGG] updated NTU = 1000.00 (tick=2036)
[AGG] updated EC = 2506.23 (tick=2036)
[AGG] updated pH = 14.58 (tick=2036)
---- Stack high-water mark report ----
  ph_task          free=1984 bytes / 3072 bytes (64.6% free)
  tds_task         free=1980 bytes / 3072 bytes (64.5% free)
  turbidity_task   free=1472 bytes / 2560 bytes (57.5% free)
  ec_task          free=1460 bytes / 2560 bytes (57.0% free)
  temp_task        free=1144 bytes / 2048 bytes (55.9% free)
[AGG] pH=14.58 TDS=827 NTU=1000.0 EC=2506 Temp=24.5
  aggregator_task  free=2092 bytes / 4096 bytes (51.1% free)
