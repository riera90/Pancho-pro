#ifndef _PANCHO_CONFIGURATION_H_
#define _PANCHO_CONFIGURATION_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../../include/spiffs.h"
#include "mqtt_handler.h"


#define TAG CONFIG_TAG


/**
 * \brief Loads the node configuration in the node
 * \details The configuration is loaded from the user defined spiffs vfile, if there is no such configuration file, the default values are loaded instead
 * \return status boolean
 * \retval TRUE the configuration was loaded from the spiffs vfile
 * \retval FALSE the configuration was loaded from the default configuration
 */
bool load_node_configuration(void) {
    // initializes the spiffs virtual file system
    spiffs_init();
    
    FILE* conf = fopen(CONFIG_SPIFFS_CONFIG_FILE, "r");
    char line[255];
    char* pos;

    if (conf == NULL) {
        // the configuration file was not found, load the default values
        ESP_LOGI(TAG, "Failed to open configuration file, fallback to default values");

        strcpy(STA_WIFI_SSID, CONFIG_STA_WIFI_SSID);
        strcpy(STA_WIFI_PASSWORD, CONFIG_STA_WIFI_PASSWORD);

        strcpy(BROKER_URL, CONFIG_BROKER_URL);
        BROKER_PORT = CONFIG_BROKER_PORT;
        strcpy(BROKER_USER, CONFIG_BROKER_USER);
        strcpy(BROKER_PASSWORD, CONFIG_BROKER_PASSWORD);
        strcpy(MQTT_SUBSCRIBED_TOPIC, CONFIG_MQTT_SUBSCRIBED_TOPIC);
        
        spiffs_end();
        return false;
    }
    // load the configration from the spiffs file
    ESP_LOGI(TAG, "Loading configuration from config file");

    fgets(STA_WIFI_SSID, sizeof(STA_WIFI_SSID), conf);
    pos = strchr(STA_WIFI_SSID, '\n'); if (pos) *pos = '\0';

    fgets(STA_WIFI_PASSWORD, sizeof(STA_WIFI_PASSWORD), conf);
    pos = strchr(STA_WIFI_PASSWORD, '\n'); if (pos) *pos = '\0';

    fgets(BROKER_URL, sizeof(BROKER_URL), conf);
    pos = strchr(BROKER_URL, '\n'); if (pos) *pos = '\0';

    fgets(line, sizeof(line), conf);
    pos = strchr(line, '\n'); if (pos) *pos = '\0';
    BROKER_PORT = atoi(line);

    fgets(BROKER_USER, sizeof(BROKER_USER), conf);
    pos = strchr(BROKER_USER, '\n'); if (pos) *pos = '\0';

    fgets(BROKER_PASSWORD, sizeof(BROKER_PASSWORD), conf);
    pos = strchr(BROKER_PASSWORD, '\n'); if (pos) *pos = '\0';

    fgets(MQTT_SUBSCRIBED_TOPIC, sizeof(MQTT_SUBSCRIBED_TOPIC), conf);
    pos = strchr(MQTT_SUBSCRIBED_TOPIC, '\n'); if (pos) *pos = '\0';

    fclose(conf);

    spiffs_end();

    return true;
}


/**
 * \brief Saves the current configuration of the node to the spiffs vfile
 * \warning the configuration does not neccesary needs to be the running configuration, until the module is not reset, it will not be set up in the current configuration
 * \return Status boolean
 * \retval TRUE success
 * \retval FALSE error, the file could not be opened
 */
bool save_node_configuration(void) {
    // initializes the spiffs virtual file system
    spiffs_init();

    FILE* conf = fopen(CONFIG_SPIFFS_CONFIG_FILE, "w");
    
    if (conf == NULL) {
        ESP_LOGE(TAG, "config: Failed to open configuration file in write mode");
        esp_vfs_spiffs_unregister(NULL);
        return false;
    }
    ESP_LOGI(TAG, "config: saving configuration to file");

    fprintf(conf, "%s\n", STA_WIFI_SSID);
    fprintf(conf, "%s\n", STA_WIFI_PASSWORD);

    // fprintf(conf, "%s\n", AP_WIFI_SSID);
    // fprintf(conf, "%s\n", AP_WIFI_PASSWORD);
    // fprintf(conf, "%i\n", AP_WIFI_STA_CONN);

    fprintf(conf, "%s\n", BROKER_URL);
    fprintf(conf, "%i\n", BROKER_PORT);
    fprintf(conf, "%s\n", BROKER_USER);
    fprintf(conf, "%s\n", BROKER_PASSWORD);
    fprintf(conf, "%s\n", MQTT_SUBSCRIBED_TOPIC);

    fclose(conf);
    spiffs_end();

    return true;
}

/**
 * \brief Removes the user defined configuration
 * \warning the running configuration will not be changed unless the node is reset
 * \return Status boolean
 * \retval TRUE success
 * \retval FALSE error, the file could not be found or deleted
 */
bool reset_node_configuration(void) {
    return delete_spiffs_file(CONFIG_SPIFFS_CONFIG_FILE);
}

#endif // _PANCHO_CONFIGURATION_H_