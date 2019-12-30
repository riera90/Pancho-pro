#ifndef _PANCHO_SPIFFS_H_
#define _PANCHO_SPIFFS_H_

#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h> 
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define TAG CONFIG_TAG


void spiffs_init(void) {
    size_t total = 0, used = 0;   
    esp_vfs_spiffs_conf_t spiffs_conf = {
      .base_path = CONFIG_SPIFFS_BASE_PATH,
      .partition_label = NULL,
      .max_files = CONFIG_SPIFFS_MAX_FILES,
      .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "spiffs: Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "spiffs: Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "spiffs: Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spiffs: Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "spiffs: Partition size: total: %d, used: %d", total, used);
    }
}

void spiffs_end(void) {
    esp_vfs_spiffs_unregister(NULL);
}


bool delete_spiffs_file(const char* path) {
    struct stat st;

    // initializes the spiffs virtual file system
    spiffs_init();

    if (stat(path, &st) == 0) {
        unlink(path);
        ESP_LOGI(TAG, "spiffs: file %s has been deleted", path);
        spiffs_end();
        return true;
    }

    ESP_LOGW(TAG, "spiffs: file %s was not found", path);
    spiffs_end();
    return false;
}

#endif // _PANCHO_SPIFFS_H_