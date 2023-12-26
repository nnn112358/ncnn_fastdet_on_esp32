
idf_component_register(SRCS "fastdet_main.cpp"
                      # EMBED_TXTFILES nanodet_m.bin nanodet_m.param #image0.rgb image1.rgb image2.rgb image0.jpg image1.jpg
                       #PRIV_REQUIRES ncnn
                       )
                       
include_directories("../ncnn/include")

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)

