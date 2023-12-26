


# Fastdet model running on ESP32

This project does not work well due to the following problems
--- The ex.extract output size is empty

```
I (10) boot: ESP-IDF v5.0.5-dirty 2nd stage bootloader
I (11) boot: compile time 06:20:07
I (11) boot: Multicore bootloader
I (13) boot: chip revision: v3.0
I (17) qio_mode: Enabling default flash chip QIO
I (22) boot.esp32: SPI Speed      : 80MHz
I (27) boot.esp32: SPI Mode       : QIO
I (32) boot.esp32: SPI Flash Size : 16MB
I (36) boot: Enabling RNG early entropy source...
I (42) boot: Partition Table:
I (45) boot: ## Label            Usage          Type ST Offset   Length
I (52) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (60) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (67) boot:  2 factory          factory app      00 00 00010000 00a00000
I (75) boot:  3 storage          Unknown data     01 82 00a10000 00400000
I (82) boot: End of partition table
I (87) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=3fa5ch (260700) map
I (167) esp_image: segment 1: paddr=0004fa84 vaddr=3ffb0000 size=00594h (  1428) load
I (167) esp_image: segment 2: paddr=00050020 vaddr=400d0020 size=e182ch (923692) map
I (426) esp_image: segment 3: paddr=00131854 vaddr=3ffb0594 size=06a50h ( 27216) load
I (435) esp_image: segment 4: paddr=001382ac vaddr=40080000 size=12b54h ( 76628) load
I (470) boot: Loaded app from partition at offset 0x10000
I (470) boot: Disabling RNG early entropy source...
I (482) cpu_start: Multicore app
I (482) quad_psram: This chip is ESP32-D0WD
I (483) esp_psram: Found 8MB PSRAM device
I (484) esp_psram: Speed: 40MHz
I (488) esp_psram: PSRAM initialized, cache is in low/high (2-core) mode.
W (495) esp_psram: Virtual address not enough for PSRAM, map as much as we can. 4MB is mapped
I (505) cpu_start: Pro cpu up.
I (508) cpu_start: Starting app cpu, entry point is 0x4008158c
0x4008158c: call_start_cpu1 at /opt/esp-idf-v5.0.5/components/esp_system/port/cpu_start.c:147

I (510) cpu_start: App cpu up.
I (1419) esp_psram: SPI SRAM memory test OK
I (1426) cpu_start: Pro cpu start user code
I (1426) cpu_start: cpu freq: 240000000 Hz
I (1427) cpu_start: Application information:
I (1430) cpu_start: Project name:     nanodet
I (1435) cpu_start: App version:      1
I (1439) cpu_start: Compile time:     Dec 27 2023 06:20:23
I (1446) cpu_start: ELF file SHA256:  7a4bd598cead1d3e...
I (1452) cpu_start: ESP-IDF:          v5.0.5-dirty
I (1457) cpu_start: Min chip rev:     v0.0
I (1462) cpu_start: Max chip rev:     v3.99 
I (1467) cpu_start: Chip rev:         v3.0
I (1472) heap_init: Initializing. RAM available for dynamic allocation:
I (1479) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (1485) heap_init: At 3FFB90A8 len 00026F58 (155 KiB): DRAM
I (1491) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (1498) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (1504) heap_init: At 40092B54 len 0000D4AC (53 KiB): IRAM
I (1511) esp_psram: Adding pool of 4096K of PSRAM memory to heap allocator
I (1519) spi_flash: detected chip: generic
I (1523) spi_flash: flash io: qio
I (1529) app_start: Starting scheduler on CPU0
I (1532) app_start: Starting scheduler on CPU1
I (1532) main_task: Started on CPU0
I (1542) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (1542) main_task: Calling app_main()
This is esp32 chip with 2 CPU core(s), WiFi/BTBLE, silicon revision v3.0, 16MB external flash
Minimum free heap size: 4433856 bytes
I (1562) fastdet_main: Initializing SPIFFS
I (1912) fastdet_main: Partition size: total: 3848081, used: 1925672
I (1912) fastdet_main: Loading model...
Minimum free heap size: 4430776 bytes
Model load_param:
Model load_model:
input_names:input.1
output_names:758
img_width320
img_height320
  ex.input size: 3, 352, 352
 ex.extract output size: 0, 0, 0
Time:1597 ms
FATAL ERROR! pool allocator destroyed too early
0x0 still in use
I (13622) main_task: Returned from app_main()

```

build command
```
idf.py build && idf.py flash && idf.py monitor 
```


## Example folder contents

This project uses fastdet & ncnn_on_esp32 as reference.

https://github.com/dog-qiuqiu/FastestDet/tree/main/example/ncnn
https://github.com/nihui/ncnn_on_esp32
https://zhuanlan.zhihu.com/p/492142807

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── README.md
├── main
│   ├── CMakeLists.txt
│   └── fastdet_main.cpp
├── ncnn
│   ├── include
│   │   └── ncnn
│   │       ├── allocator.h
│   │       ├── benchmark.h
│   │       ├── blob.h
│   │       ├── c_api.h
│   │       ├── command.h
│   │       ├── cpu.h
│   │       ├── datareader.h
│   │       ├── gpu.h
│   │       ├── layer.h
│   │       ├── layer_shader_type.h
│   │       ├── layer_shader_type_enum.h
│   │       ├── layer_type.h
│   │       ├── layer_type_enum.h
│   │       ├── mat.h
│   │       ├── modelbin.h
│   │       ├── ncnn_export.h
│   │       ├── net.h
│   │       ├── option.h
│   │       ├── paramdict.h
│   │       ├── pipeline.h
│   │       ├── pipelinecache.h
│   │       ├── platform.h
│   │       ├── simplemath.h
│   │       ├── simpleocv.h
│   │       ├── simpleomp.h
│   │       ├── simplestl.h
│   │       └── vulkan_header_fix.h
│   └── lib
│       ├── cmake
│       │   └── ncnn
│       │       ├── ncnn-release.cmake
│       │       ├── ncnn.cmake
│       │       └── ncnnConfig.cmake
│       ├── libncnn.a
│       └── pkgconfig
│           └── ncnn.pc
├── partitions.csv
├── sdkconfig
├── sdkconfig.defaults
└── spiffs_image
    ├── 3.jpg
    ├── FastestDet.bin
    ├── FastestDet.param
    ├── image0.jpg
    ├── image1.jpg
    └── image3.png

9 directories, 45 files

```

