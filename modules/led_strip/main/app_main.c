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

#define CONCAT );strcat(resp_str, 
#define ENDCONCAT );strcat(resp_str,

#define PWM_0_OUT_IO_NUM 15 // red
#define PWM_1_OUT_IO_NUM 12 // green
#define PWM_2_OUT_IO_NUM 13 // blue

#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD          (1000)

#define TAG CONFIG_TAG

bool load_node_configuration(void) {
    // initializes the spiffs virtual file system
    spiffs_init();

    FILE* conf = fopen("/spiffs/configuration.txt", "r");
    if (conf == NULL) {
        ESP_LOGI(TAG, "Failed to open configuration file, fallback to default values");

        strcpy(STA_WIFI_SSID, CONFIG_STA_WIFI_SSID);
        strcpy(STA_WIFI_PASSWORD, CONFIG_STA_WIFI_PASSWORD);
        
        // strcpy(AP_WIFI_SSID, CONFIG_AP_WIFI_SSID);
        // strcpy(AP_WIFI_PASSWORD, CONFIG_AP_WIFI_PASSWORD);
        // AP_WIFI_STA_CONN = CONFIG_AP_WIFI_STA_CONN;

        strcpy(BROKER_URL, CONFIG_BROKER_URL);
        BROKER_PORT = CONFIG_BROKER_PORT;
        strcpy(BROKER_USER, CONFIG_BROKER_USER);
        strcpy(BROKER_PASSWORD, CONFIG_BROKER_PASSWORD);
        
        spiffs_end();
        return false;
    }
    ESP_LOGI(TAG, "Loading configuration from config file");

    int size = 255;
    char* line[255];
    char* pos;

    fgets(STA_WIFI_SSID, sizeof(STA_WIFI_SSID), conf);
    pos = strchr(STA_WIFI_SSID, '\n'); if (pos) *pos = '\0';

    fgets(STA_WIFI_PASSWORD, sizeof(STA_WIFI_PASSWORD), conf);
    pos = strchr(STA_WIFI_PASSWORD, '\n'); if (pos) *pos = '\0';


    // fgets(AP_WIFI_SSID, sizeof(AP_WIFI_SSID), conf);
    // pos = strchr(AP_WIFI_SSID, '\n'); if (pos) *pos = '\0';

    // fgets(AP_WIFI_PASSWORD, sizeof(AP_WIFI_PASSWORD), conf);
    // pos = strchr(AP_WIFI_PASSWORD, '\n'); if (pos) *pos = '\0';

    // fgets(line, sizeof(line), conf);
    // pos = strchr(line, '\n'); if (pos) *pos = '\0';
    // AP_WIFI_STA_CONN = atoi(line);


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

bool save_node_configuration(void) {
    // initializes the spiffs virtual file system
    spiffs_init();

    FILE* conf = fopen("/spiffs/configuration.txt", "w");
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


bool reset_node_configuration(void) {
    // initializes the spiffs virtual file system
    spiffs_init();

    struct stat st;
    if (stat("/spiffs/configuration.txt", &st) == 0) {
        unlink("/spiffs/configuration.txt");
        ESP_LOGI(TAG, "config: configuration has been reset");
        spiffs_end();
        return true;
    }

    ESP_LOGE(TAG, "config: configuration file was not found while reset");
    spiffs_end();
    return false;
}


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
            if (param[i+1] == 'A' && param[i+2] == '3')
                param[i] = '€';
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
    save_node_configuration();
    return ESP_OK;
}

esp_err_t configuration_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "web: building web page");
    char broker_port_char[8];
    itoa(BROKER_PORT, broker_port_char, 10);
    ESP_LOGI(TAG,"port string %s", broker_port_char);
    ESP_LOGI(TAG,"port int %i", BROKER_PORT);
    const char resp_str [1024] = "\0";
    strcat(resp_str,
        "<meta charset='utf-8'>"\
        "<style>"\
            "label {display: inline-block; width: 150px; text-align: right;}"\
        "</style>"\
        "<form method='post'>"\
            "<div>"\
                "<label for='STA_WIFI_SSID'>wifi name:</label>"\
                "<input type='text' name='STA_WIFI_SSID' value='"CONCAT STA_WIFI_SSID ENDCONCAT"'>"\
            "</div>"\
            "<div>"\
                "<label for='STA_WIFI_PASSWORD'>wifi password:</label>"\
                "<input type='text' name='STA_WIFI_PASSWORD' value='"CONCAT STA_WIFI_PASSWORD ENDCONCAT"'>"\
            "</div>"\
            "<div>"\
                "<label for='BROKER_URL'>mqtt server ip:</label>"\
                "<input type='text' name='BROKER_URL' value='"CONCAT BROKER_URL ENDCONCAT"'>"\
            "</div>"\
            "<div>"\
                "<label for='BROKER_PORT'>mqtt server port:</label>"\
                "<input type='number' name='BROKER_PORT' value='"CONCAT broker_port_char ENDCONCAT"'>"\
            "</div>"\
            "<div>"\
                "<label for='BROKER_USER'>mqtt username:</label>"\
                "<input type='text' name='BROKER_USER' value='"CONCAT BROKER_USER ENDCONCAT"'>"\
            "</div>"\
            "<div>"\
                "<label for='BROKER_PASSWORD'>mqtt password:</label>"\
                "<input type='text' name='BROKER_PASSWORD' value='"CONCAT BROKER_PASSWORD ENDCONCAT"'>"\
            "</div>"\
            "<div>"\
                "<input type='submit' value='Submit'>"\
            "</div>"\
        "</form>"
    );
    strcat(resp_str,
        req->user_ctx
    );
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

uint32_t new_duties[3] = {0, 0, 0};

int16_t phase[3] = {
    0, 0, 0
};

void change_to_new_pwm_duties() {
    int32_t difference_per_cicle[3];
    for (size_t i = 0; i < 3; i++)
        difference_per_cicle[i] = (int32_t) (((int16_t)new_duties[i] - (int16_t)duties[i]) / 50);

    ESP_LOGI(TAG, "difference_per_cicle: r:%i g:%i g:%i", difference_per_cicle[0], difference_per_cicle[1], difference_per_cicle[2]);
    
    for (size_t i = 0; i < 50; i++)
    {
        for (size_t i = 0; i < 3; i++)
            duties[i] += difference_per_cicle[i];
        
        pwm_set_duties(duties);
        pwm_start(); // restart them with new configuration
        ESP_LOGI(TAG, "pwm duties: r:%u g:%u g:%u", duties[0], duties[1], duties[2]);
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


void app_main() {
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

    // reset_node_configuration();

    if (load_node_configuration()) {
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