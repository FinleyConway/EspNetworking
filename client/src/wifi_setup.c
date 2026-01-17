#include "wifi_setup.h"

#include <esp_wifi.h>
#include <esp_log.h>

#include "wifi_creds.h"

// ===================== Private ===================== 
static EventGroupHandle_t s_wifi_event_group; // event group to contain status information
static int s_retry_num = 0; // retry tracker

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base != WIFI_EVENT) return;

    if (event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(WIFI_LOG_TAG, "Connecting to AP...");
        esp_wifi_connect();
    }

    if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_FAILURES) {
			ESP_LOGI(WIFI_LOG_TAG, "Reconnecting to AP...");

            // re-attempt to connect back
			esp_wifi_connect();
			s_retry_num++;
		} 
        else {
            // failed to connect after too many rety attempts
			xEventGroupSetBits(s_wifi_event_group, NET_WIFI_FAILURE);
		}
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // has retrieved a ip from the network
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        // log connected ip address
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_LOG_TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));

        // clear rety attempts
        s_retry_num = 0;

        // connected
        xEventGroupSetBits(s_wifi_event_group, NET_WIFI_SUCCESS);
    }

}
// ===================== Private ===================== 

net_status_t connect_to_wifi(void)
{
	net_status_t status = NET_WIFI_FAILURE;

	ESP_ERROR_CHECK(esp_netif_init()); // initialize the esp network interface
	ESP_ERROR_CHECK(esp_event_loop_create_default()); // initialize default esp event loop

	esp_netif_create_default_wifi_sta(); // create wifi station in the wifi driver

	//setup wifi station with the default wifi configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // create an event loop to handle events when handling the wifi driver
	s_wifi_event_group = xEventGroupCreate();

    // create handlers to close events when done
    esp_event_handler_instance_t wifi_handler_event_instance;
    esp_event_handler_instance_t got_ip_event_instance;

    // set up event for wifi specific events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        &wifi_handler_event_instance
    ));

    // set up event for receiving an ip
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &ip_event_handler,
        NULL,
        &got_ip_event_instance
    ));

    wifi_config_t wifi_config = get_wifi_config();

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // set the wifi controller to be a station
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // set the wifi config
    ESP_ERROR_CHECK(esp_wifi_start()); // start the wifi driver

    ESP_LOGI(WIFI_LOG_TAG, "STA initialization complete");

    // block thread until an wifi connection status occurs
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        NET_WIFI_SUCCESS | NET_WIFI_FAILURE,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    // evalute the bits that were returned when attempting to connect
    if (bits & NET_WIFI_SUCCESS) {
        ESP_LOGI(WIFI_LOG_TAG, "Connected to ap");
        status = NET_WIFI_SUCCESS;
    } 
    else if (bits & NET_WIFI_FAILURE) {
        ESP_LOGI(WIFI_LOG_TAG, "Failed to connect to ap");
        status = NET_WIFI_FAILURE;
    } 
    else {
        ESP_LOGE(WIFI_LOG_TAG, "UNEXPECTED EVENT");
        status = NET_WIFI_FAILURE;
    }

    // unregister events using the handlers created for setting up wifi
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(s_wifi_event_group);

    return status;
}