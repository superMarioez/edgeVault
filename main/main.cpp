#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "edgevault";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "EdgeVault v0.1.0");
}
