

#include <float.h>
#include <ncnn/net.h>
#include <ncnn/simpleocv.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "esp_spiffs.h"
#include "esp_timer.h"
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <inttypes.h>
#include <iostream>
#include <sdkconfig.h>
#include <stdio.h>
using namespace std;
using namespace cv;

static const char *TAG = "fastdet_main";

float Sigmoid(float x) {
    return 1.0f / (1.0f + exp(-x));
}

float Tanh(float x) {
    return 2.0f / (1.0f + exp(-2 * x)) - 1;
}

class TargetBox {
  private:
    float GetWidth() { return (x2 - x1); };
    float GetHeight() { return (y2 - y1); };

  public:
    int x1;
    int y1;
    int x2;
    int y2;

    int category;
    float score;

    float area() { return GetWidth() * GetHeight(); };
};

float IntersectionArea(const TargetBox &a, const TargetBox &b) {
    if(a.x1 > b.x2 || a.x2 < b.x1 || a.y1 > b.y2 || a.y2 < b.y1) {
        // no intersection
        return 0.f;
    }

    float inter_width = std::min(a.x2, b.x2) - std::max(a.x1, b.x1);
    float inter_height = std::min(a.y2, b.y2) - std::max(a.y1, b.y1);

    return inter_width * inter_height;
}

bool scoreSort(TargetBox a, TargetBox b) {
    return (a.score > b.score);
}

int nmsHandle(std::vector<TargetBox> &src_boxes, std::vector<TargetBox> &dst_boxes) {
    std::vector<int> picked;

    sort(src_boxes.begin(), src_boxes.end(), scoreSort);

    for(int i = 0; i < src_boxes.size(); i++) {
        int keep = 1;
        for(int j = 0; j < picked.size(); j++) {
            float inter_area = IntersectionArea(src_boxes[i], src_boxes[picked[j]]);
            float union_area = src_boxes[i].area() + src_boxes[picked[j]].area() - inter_area;
            float IoU = inter_area / union_area;

            if(IoU > 0.45 && src_boxes[i].category == src_boxes[picked[j]].category) {
                keep = 0;
                break;
            }
        }

        if(keep) {
            picked.push_back(i);
        }
    }

    for(int i = 0; i < picked.size(); i++) {
        dst_boxes.push_back(src_boxes[picked[i]]);
    }

    return 0;
}

extern "C" void app_main(void) {

    static const char *class_names[] = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush"};
    int class_num = sizeof(class_names) / sizeof(class_names[0]);

    float thresh = 0.65;

    int input_width = 352;
    int input_height = 352;

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if(ret != ESP_OK) {
        if(ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if(ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    esp_err_t ret2 = esp_spiffs_info(NULL, &total, &used);
    if(ret2 != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    ESP_LOGI(TAG, "Loading model...");

    ncnn::Net net;
    net.opt.lightmode = true;
    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    cout << "Model load_param:" << endl;
    if(net.load_param("/spiffs/FastestDet.param")) {
        ESP_LOGI(TAG, "Model load_param..error");
        return ;
    }

    cout << "Model load_model:" << endl;
    if(net.load_model("/spiffs/FastestDet.bin")) {
        ESP_LOGI(TAG, "Model load_bin..error");
         return ;
    }

    const std::vector<const char *> &input_names = net.input_names();
    const std::vector<const char *> &output_names = net.output_names();

    for(size_t i = 0; i < input_names.size(); i++) {
        cout << "input_names:" << input_names[i] << endl;
    }
    for(size_t i = 0; i < output_names.size(); i++) {
        cout << "output_names:" << output_names[i] << endl;
    }

    cv::Mat img = cv::imread("/spiffs/image0.jpg");
    int img_width = img.cols;
    int img_height = img.rows;
    cout << "img_width" << img_width << endl;
    cout << "img_height" << img_height << endl;

    // resize of input image data
    ncnn::Mat input = ncnn::Mat::from_pixels_resize(img.data, ncnn::Mat::PIXEL_BGR,
                                                    img.cols, img.rows, input_width, input_height);

    // Normalization of input image data
    const float mean_vals[3] = {0.f, 0.f, 0.f};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    input.substract_mean_normalize(mean_vals, norm_vals);

    // creat extractor
    ncnn::Extractor ex = net.create_extractor();

    long start = esp_timer_get_time();
    // set input tensor
    ex.input(input_names[0], input);
    printf("  ex.input size: %d, %d, %d\n", input.c, input.h, input.w);

    // get output tensor
    ncnn::Mat output;
    ex.extract(output_names[0], output);
    printf(" ex.extract output size: %d, %d, %d\n", output.c, output.h, output.w);

    // handle output tensor
    std::vector<TargetBox> target_boxes;

    for(int h = 0; h < output.h; h++) {
        for(int w = 0; w < output.h; w++) {
            int obj_score_index = (0 * output.h * output.w) + (h * output.w) + w;
            float obj_score = output[obj_score_index];

            int category;
            float max_score = 0.0f;
            for(size_t i = 0; i < class_num; i++) {
                int obj_score_index = ((5 + i) * output.h * output.w) + (h * output.w) + w;
                float cls_score = output[obj_score_index];
                if(cls_score > max_score) {
                    max_score = cls_score;
                    category = i;
                }
            }
            float score = pow(max_score, 0.4) * pow(obj_score, 0.6);

            if(score > thresh) {
                int x_offset_index = (1 * output.h * output.w) + (h * output.w) + w;
                int y_offset_index = (2 * output.h * output.w) + (h * output.w) + w;
                int box_width_index = (3 * output.h * output.w) + (h * output.w) + w;
                int box_height_index = (4 * output.h * output.w) + (h * output.w) + w;

                float x_offset = Tanh(output[x_offset_index]);
                float y_offset = Tanh(output[y_offset_index]);
                float box_width = Sigmoid(output[box_width_index]);
                float box_height = Sigmoid(output[box_height_index]);

                float cx = (w + x_offset) / output.w;
                float cy = (h + y_offset) / output.h;

                int x1 = (int)((cx - box_width * 0.5) * img_width);
                int y1 = (int)((cy - box_height * 0.5) * img_height);
                int x2 = (int)((cx + box_width * 0.5) * img_width);
                int y2 = (int)((cy + box_height * 0.5) * img_height);

                target_boxes.push_back(TargetBox{x1, y1, x2, y2, category, score});
            }
        }
    }

    std::vector<TargetBox> nms_boxes;
    nmsHandle(target_boxes, nms_boxes);
    long end = esp_timer_get_time();

    long time = end - start;
    printf("Time:%ld ms\n", time);

    // draw result
    for(size_t i = 0; i < nms_boxes.size(); i++) {
        TargetBox box = nms_boxes[i];
        printf("x1:%d y1:%d x2:%d y2:%d  %s:%.2f%%\n", box.x1, box.y1, box.x2, box.y2, class_names[box.category], box.score * 100);
    }

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    return;
}
