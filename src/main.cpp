#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_http_client.h>
#include <esp_wifi.h>
#include <esp_https_ota.h>

#include <version.h>
#include "secret.h"

#define ENABLE_OTA
#ifdef ENABLE_OTA
extern "C" void wifi_init_sta();
#endif
#ifndef WIFI_PWD
#error Missing WIFI_PWD Environment variable
#endif
#ifndef WIFI_SSID
#error Missing WIFI_SSID Environment variable
#endif

static char TAG[] = "MAIN";

extern "C" void app_main();

static TaskHandle_t core_A = NULL;
static TaskHandle_t core_B = NULL;

void EnableWifi()
{
#ifdef ENABLE_OTA
    wifi_init_sta();
#endif
}

void updateOTA()
{
    ESP_LOGI(TAG, "Starting OTA task");
    esp_http_client_config_t config = {
        .url = "http://192.168.0.16:8000/firmware.bin?",
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
}

void checkVersionFW()
{
    char newVersionFW[50];
    memset(newVersionFW, 0, 50);

    esp_http_client_config_t configFW = {
        .url = "http://192.168.0.16:8000/VERSION.html",  //CAMBIAR POR LA URL DE TU SERVIDOR
        .user_data = newVersionFW,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&configFW);
    esp_http_client_open(client,0);
    esp_http_client_fetch_headers(client);
    esp_http_client_read(client, newVersionFW, esp_http_client_get_content_length(client));

    ESP_LOGE(TAG,"Received html: %s", newVersionFW);
    uint64_t value;
    value = atoll(newVersionFW);
    ESP_LOGE(TAG,"Received html: %llu", value);

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

   

    if (value > actualVersion)
    {   
        ESP_LOGE(TAG, "NEW VERSION!!!");
        updateOTA();
    }
}

void coreAThread(void *arg)
{
     ESP_LOGE(TAG, "Iniciando CORE A");  
    while(1)
    {
        
    }
}

void coreBThread(void *arg)
{
     ESP_LOGE(TAG, "Iniciando CORE B");  
    while(1)
    {
        
    }
}

void app_main()
{   
    ESP_LOGE(TAG, "Iniciando software");
    if (esp_wifi_connect() == ESP_OK)
    {
        checkVersionFW();
    }

    xTaskCreatePinnedToCore(coreBThread, "core_B", 4096, NULL, 9, &core_B, 1);    
    xTaskCreatePinnedToCore(coreAThread, "core_A", 4096, NULL, 10, &core_A, 0);
    
}