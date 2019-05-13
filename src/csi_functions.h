
#include "esp_wifi.h"
#include "config.h"
#include "WiFi_Functions.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdio.h>
#include <Arduino.h>
#include "structures.h"

extern unsigned long time_out_csi;
extern esp_err_t ret;
extern wifi_second_chan_t ht_chan;
void setup_wifi_csi();
//const char * wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type);
extern int conta_csi;
extern bool begin_csi_scan;
extern bool send_csi_message;
extern int curChannel;
extern String csiMessage;
#ifdef STATIC_CHANNEL
  extern bool set_static_channel;
#endif