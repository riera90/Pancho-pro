#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/pwm.h"

#include "../../include/mqtt.h"
#include "../../include/wireless.h"
#include "../../include/spiffs.h"
#include "../../include/web_server.h"

#define PWM_0_OUT_IO_NUM 15 // red
#define PWM_1_OUT_IO_NUM 12 // green
#define PWM_2_OUT_IO_NUM 13 // blue

#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD          (1000)

#define TAG CONFIG_TAG

/**
 * \brief Loads the node configuration in the node
 * \details The configuration is loaded from the user defined spiffs vfile, if there is no such configuration file, the default values are loaded instead
 * \param path Path to the configuration spiffs vfile
 * \return status boolean
 * \retval TRUE the configuration was loaded from the spiffs vfile
 * \retval FALSE the configuration was loaded from the default configuration
 */
bool load_node_configuration(const char* path) {
    // initializes the spiffs virtual file system
    spiffs_init();

    FILE* conf = fopen(path, "r");
    if (conf == NULL) {
        // the configuration file was not found, load the default values
        ESP_LOGI(TAG, "Failed to open configuration file, fallback to default values");

        strcpy(STA_WIFI_SSID, CONFIG_STA_WIFI_SSID);
        strcpy(STA_WIFI_PASSWORD, CONFIG_STA_WIFI_PASSWORD);

        strcpy(BROKER_URL, CONFIG_BROKER_URL);
        BROKER_PORT = CONFIG_BROKER_PORT;
        strcpy(BROKER_USER, CONFIG_BROKER_USER);
        strcpy(BROKER_PASSWORD, CONFIG_BROKER_PASSWORD);
        
        spiffs_end();
        return false;
    }
    // load the configration from the spiffs file
    ESP_LOGI(TAG, "Loading configuration from config file");

    char line[255];
    char* pos;

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

    fclose(conf);

    spiffs_end();

    return true;
}


/**
 * \brief Saves the current configuration of the node to the spiffs vfile
 * \warning the configuration does not neccesary needs to be the running configuration, until the module is not reset, it will not be set up in the current configuration
 * \param path Path to the configuration spiffs vfile
 * \return Status boolean
 * \retval TRUE success
 * \retval FALSE error, the file could not be opened
 */
bool save_node_configuration(const char* path) {
    // initializes the spiffs virtual file system
    spiffs_init();

    FILE* conf = fopen(path, "w");
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

    fclose(conf);
    spiffs_end();

    return true;
}

/**
 * \brief Removes the user defined configuration
 * \warning the running configuration will not be changed unless the node is reset
 * \param path Path to the configuration spiffs vfile
 * \return Status boolean
 * \retval TRUE success
 * \retval FALSE error, the file could not be found or deleted
 */
bool reset_node_configuration(const char* path) {
    // initializes the spiffs virtual file system
    spiffs_init();

    struct stat st;
    if (stat(path, &st) == 0) {
        unlink(path);
        ESP_LOGI(TAG, "config: configuration has been reset");
        spiffs_end();
        return true;
    }

    ESP_LOGE(TAG, "config: configuration file was not found while reset");
    spiffs_end();
    return false;
}

/**
 * \brief decodes a string from the http standar i utf8 to utf8
 * \return new string
 * \warning the string length could be changed
 */
char* decode_web_param(char * param) {
    size_t max = strlen(param);
    size_t i = 0;
    for (; i < max;i++) {
        if (param[i] == '\0')
            break;

        if (param[i] == '+') {
            param[i] = ' ';
        }

        if (param[i] == '%') {
            if (param[i+1] == '2' && param[i+2] == '1')
                param[i] = '!';
            if (param[i+1] == '2' && param[i+2] == '2')
                param[i] = '"';
            if (param[i+1] == '2' && param[i+2] == '3')
                param[i] = '#';
            if (param[i+1] == '2' && param[i+2] == '4')
                param[i] = '$';
            if (param[i+1] == '2' && param[i+2] == '5')
                param[i] = '%';
            if (param[i+1] == '2' && param[i+2] == '6')
                param[i] = '&';
            if (param[i+1] == '2' && param[i+2] == '7')
                param[i] = '\'';
            if (param[i+1] == '2' && param[i+2] == '8')
                param[i] = '(';
            if (param[i+1] == '2' && param[i+2] == '9')
                param[i] = ')';
            if (param[i+1] == '2' && param[i+2] == 'B')
                param[i] = '+';
            if (param[i+1] == '2' && param[i+2] == 'C')
                param[i] = ',';
            if (param[i+1] == '2' && param[i+2] == 'F')
                param[i] = '/';
            if (param[i+1] == '3' && param[i+2] == 'A')
                param[i] = ':';
            if (param[i+1] == '3' && param[i+2] == 'B')
                param[i] = ';';
            if (param[i+1] == '3' && param[i+2] == 'C')
                param[i] = '<';
            if (param[i+1] == '3' && param[i+2] == 'E')
                param[i] = '>';
            if (param[i+1] == '3' && param[i+2] == 'D')
                param[i] = '=';
            if (param[i+1] == '3' && param[i+2] == 'F')
                param[i] = '?';
            if (param[i+1] == '4' && param[i+2] == '0')
                param[i] = '@';
            if (param[i+1] == '5' && param[i+2] == 'B')
                param[i] = '[';
            if (param[i+1] == '5' && param[i+2] == 'D')
                param[i] = ']';
            if (param[i+1] == '5' && param[i+2] == 'E')
                param[i] = '^';
            if (param[i+1] == '5' && param[i+2] == 'C')
                param[i] = '\\';
            if (param[i+1] == '7' && param[i+2] == 'E')
                param[i] = '~';
            //if (param[i+1] == 'A' && param[i+2] == '3')
            //    param[i] = "€";
            max-=2;
            for (size_t j = i+1; j < max; j++)
                param[j] = param[j+2];
            param[max]='\0';
        }
    }
    return param;
}


void handle_config_parameter(char* param, char* value) {
    param = decode_web_param(param);
    value = decode_web_param(value);
    if (strcmp(param, "STA_WIFI_SSID") == 0)
        strcpy(STA_WIFI_SSID, value);
    
    if (strcmp(param, "STA_WIFI_PASSWORD") == 0)
        strcpy(STA_WIFI_PASSWORD, value);

    if (strcmp(param, "BROKER_URL") == 0)
        strcpy(BROKER_URL, value);

    if (strcmp(param, "BROKER_PORT") == 0)
        BROKER_PORT = atoi(value);

    if (strcmp(param, "BROKER_USER") == 0)
        strcpy(BROKER_USER, value);
 
    if (strcmp(param, "BROKER_PASSWORD") == 0)
        strcpy(BROKER_PASSWORD, value);

    ESP_LOGI(TAG, "param: %s:%s", param, value);
}


/* An HTTP POST handler */
esp_err_t configuration_post_handler(httpd_req_t *req) {
    char buf[1024];
    bzero(buf, 1024);
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED CONFIG DATA ===========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "============================================");
    }


    char* pch;
    char param[25];
    char value[75];
    pch = strtok(buf, "=");
    strcpy(param, pch);
    pch = strtok(NULL, "&");
    strcpy(value, pch);
    while (param != NULL && value != NULL) {
        handle_config_parameter(param, value);
        pch = strtok(NULL, "=");
        if (pch == NULL)
            break;
        strcpy(param, pch);
        pch = strtok(NULL, "&");
        strcpy(value, pch);
    }
    char response[100] = "The configuration was saved, please reload the page to change them again.";

    httpd_resp_send(req, response, strlen(response));
    // httpd_resp_send_chunk(req, response, strlen(response));
    save_node_configuration("/spiffs/configuration.txt");
    return ESP_OK;
}

esp_err_t configuration_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "web: building web page");
    char broker_port_char[8];
    itoa(BROKER_PORT, broker_port_char, 10);

    char resp_str [1024];
    char resp_str_aux [255];

    strcpy(resp_str, (char * restrict)"<meta charset='utf-8'><style>label {display: inline-block; width: 150px; text-align: right;}</style><form method='post'><div><label for='STA_WIFI_SSID'>wifi name:</label><input type='text' name='STA_WIFI_SSID' value='\0");
    strcat(resp_str, STA_WIFI_SSID);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='STA_WIFI_PASSWORD'>wifi password:</label><input type='text' name='STA_WIFI_PASSWORD' value='\0");
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, STA_WIFI_PASSWORD);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_URL'>mqtt server ip:</label><input type='text' name='BROKER_URL' value='\0");
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, BROKER_URL);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_PORT'>mqtt server port:</label><input type='number' name='BROKER_PORT' value='\0");
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, broker_port_char);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_USER'>mqtt username:</label><input type='text' name='BROKER_USER' value='\0");
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, BROKER_USER);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_PASSWORD'>mqtt password:</label><input type='text' name='BROKER_PASSWORD' value='\0");
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, BROKER_PASSWORD);

    strcpy(resp_str_aux, (char * restrict)"'></div><div><input type='submit' value='Submit'></div></form>\0");
    strcat(resp_str, resp_str_aux);

    /*
    the web page:

    <meta charset='utf-8'>
    <style>
        label {display: inline-block; width: 150px; text-align: right;}
    </style>
    <form method='post'>
        <div>
            <label for='STA_WIFI_SSID'>wifi name:</label>
            <input type='text' name='STA_WIFI_SSID' value='"STA_WIFI_SSID"'>
        </div>
        <div>
            <label for='STA_WIFI_PASSWORD'>wifi password:</label>
            <input type='text' name='STA_WIFI_PASSWORD' value='"STA_WIFI_PASSWORD"'>
        </div>
        <div>
            <label for='BROKER_URL'>mqtt server ip:</label>
            <input type='text' name='BROKER_URL' value='"BROKER_URL"'>
        </div>
        <div>
            <label for='BROKER_PORT'>mqtt server port:</label>
            <input type='number' name='BROKER_PORT' value='"broker_port_char"'>
        </div>
        <div>
            <label for='BROKER_USER'>mqtt username:</label>
            <input type='text' name='BROKER_USER' value='"BROKER_USER"'>
        </div>
        <div>
            <label for='BROKER_PASSWORD'>mqtt password:</label>
            <input type='text' name='BROKER_PASSWORD' value='"BROKER_PASSWORD"'>
        </div>
        <div>
            <input type='submit' value='Submit'>
        </div>
    </form>"
    */
    
    
    ESP_LOGI(TAG, "web: sending web page");

    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

httpd_uri_t configuration_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = configuration_handler,
    .user_ctx  = "Node Configuration"
};

httpd_uri_t configuration_post_uri = {
    .uri       = "/",
    .method    = HTTP_POST,
    .handler   = configuration_post_handler,
    .user_ctx  = "Node Configuration post"
};

const uint32_t pin_num[3] = {
    PWM_0_OUT_IO_NUM,
    PWM_1_OUT_IO_NUM,
    PWM_2_OUT_IO_NUM,
};

uint32_t duties[3] = {
    0, 0, 0
};

uint32_t new_duties[3] = {
    0, 0, 0
};

int16_t phase[3] = {
    0, 0, 0
};

void change_to_new_pwm_duties(void) {
    int32_t difference_per_cicle[3];
    for (size_t i = 0; i < 3; i++)
        difference_per_cicle[i] = (int32_t) (((int16_t)new_duties[i] - (int16_t)duties[i]) / 50);

    ESP_LOGI(TAG, "difference_per_cicle: r:%i g:%i b:%i", difference_per_cicle[0], difference_per_cicle[1], difference_per_cicle[2]);
    
    for (size_t i = 0; i < 50; i++)
    {
        for (size_t i = 0; i < 3; i++)
            duties[i] += difference_per_cicle[i];
        
        pwm_set_duties(duties);
        pwm_start(); // restart them with new configuration
        ESP_LOGI(TAG, "pwm duties: r:%u g:%u b:%u", duties[0], duties[1], duties[2]);
        vTaskDelay( (2000/50) / portTICK_RATE_MS);
    }
    
    for (size_t i = 0; i < 3; i++)
        duties[i] = new_duties[i];
    pwm_set_duties(duties);
    pwm_start(); // restart them with new configuration
}

void command_handler(esp_mqtt_event_handle_t event) {
    
    char* buffer = (char*)calloc((event->data_len/sizeof(char)) + 1, sizeof(char));

    for (size_t i = 0; i < event->data_len/sizeof(char); i++) 
        buffer[i] = event->data[i];
    buffer[event->data_len/sizeof(char)] = '\0';
        
    char* pch;
    pch = strtok(buffer, ",");
    size_t index = 0;
    while (pch != NULL && index < 3)
    {
        new_duties[index] = (uint32_t)atoi(pch);
        index++;
        pch = strtok (NULL, ",");
    }
    free(buffer);
    for (size_t i = 0; i < 3; i++) {
        if (new_duties[i] > PWM_PERIOD) {
            new_duties[i] = (uint32_t)PWM_PERIOD;
            ESP_LOGE(TAG, "PWM: the duty can't be more than the period");
        }
    }
    ESP_LOGI(TAG, "new pwm duties: r:%u g:%u g:%u", new_duties[0], new_duties[1], new_duties[2]);
    change_to_new_pwm_duties();
    ESP_LOGI(TAG, "pwm duties: r:%u g:%u g:%u", duties[0], duties[1], duties[2]);
}

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client) {
    mqtt_subscribe_to_topic(client, CONFIG_MQTT_SUBSCRIBED_TOPIC, CONFIG_BROKER_QOS);
}


void app_main(void) {
    pwm_init(PWM_PERIOD, duties, 3, pin_num);
    pwm_set_phases(phase);
    pwm_start();

    ESP_LOGI(TAG, "Startup..");
    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    reset_node_configuration("/spiffs/configuration.txt");

    if (load_node_configuration("/spiffs/configuration.txt")) {
        ESP_LOGI(TAG, "entering sta conf");
        wifi_init_sta();
        mqtt_init();
    } else {
        ESP_LOGI(TAG, "entering softap conf");
        wifi_init_softap();
        httpd_uri_t* uris[2] = {&configuration_uri, &configuration_post_uri};
        config_web_server_init(uris, 2);
    }
    
}