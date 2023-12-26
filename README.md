# ncnn & Fastdet model running on ESP32

This project does not work well due to the following problems
---> The ex.extract() output size is empty

```
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
│   └── lib
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

