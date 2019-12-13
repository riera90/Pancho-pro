#ifndef _PANCHO_WIRELESS_H_
#define _PANCHO_WIRELESS_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG CONFIG_TAG

char STA_WIFI_SSID[255];
char STA_WIFI_PASSWORD[255];

// char AP_WIFI_SSID[255];
// char AP_WIFI_PASSWORD[255];
// int AP_WIFI_STA_CONN;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;


static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch(event->event_id) {
        // wifi has been disconnected
        case SYSTEM_EVENT_STA_START: 
            esp_wifi_connect();
            break;

        // node has got an ip
        case SYSTEM_EVENT_STA_GOT_IP: 
            ESP_LOGI(TAG, "wifi: got ip:%s",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;

        // node has started a connection
        case SYSTEM_EVENT_AP_STACONNECTED: 
            ESP_LOGI(TAG, "wifi: station:"MACSTR" join, AID=%d",
            MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
            break;

        // unexpected ap disconnect
        case SYSTEM_EVENT_AP_STADISCONNECTED: 
            ESP_LOGI(TAG, "wifi: station:"MACSTR" leave, AID=%d",
            MAC2STR(event->event_info.sta_disconnected.mac), event->event_info.sta_disconnected.aid);
            break;

        // the node has lose connection to the station
        case SYSTEM_EVENT_STA_DISCONNECTED: 
            ESP_LOGE(TAG, "wifi: Disconnect reason: %d", info->disconnected.reason);
            if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
                /*Switch to 802.11 bgn mode */
                esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
            }
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;

        // unexpected error
        default:
            ESP_LOGI(TAG, "wifi: unhandled event, event id: %i", event->event_id);
            break;
    }
    return ESP_OK;
}


void wifi_init_softap() {
    wifi_event_group = xEventGroupCreate();
    wifi_config_t wifi_config = {0};

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    
    strcpy((char *)wifi_config.ap.ssid, CONFIG_AP_WIFI_SSID);
    strcpy((char *)wifi_config.ap.password, CONFIG_AP_WIFI_PASSWORD);
    wifi_config.ap.ssid_len = strlen(CONFIG_AP_WIFI_SSID);
    wifi_config.ap.max_connection = CONFIG_AP_WIFI_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(CONFIG_AP_WIFI_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished as SSID:%s password:%s",
             CONFIG_AP_WIFI_SSID, CONFIG_AP_WIFI_PASSWORD);
}


void wifi_init_sta() {
    wifi_event_group = xEventGroupCreate();
    wifi_config_t wifi_config = {0};

    tcpip_adapter_init();
    
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    strcpy((char *)wifi_config.ap.ssid, STA_WIFI_SSID);
    strcpy((char *)wifi_config.ap.password, STA_WIFI_PASSWORD);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi: wifi_init_sta finished.");
    ESP_LOGI(TAG, "wifi: connect to ap SSID:%s password:%s", STA_WIFI_SSID, STA_WIFI_PASSWORD);
}

#endif // _PANCHO_WIRELESS_H_